#include <vector>
#include <unordered_map>
#include <set>
#include <tuple>
#include <algorithm>
#include <memory>
#include <iostream>
#include <numeric>
#include <boost/lockfree/spsc_queue.hpp>
#include "state_orchestrator.hpp"
#include "utils.hpp"
#include "registry.hpp"
#include "expiry_state.hpp"
#include "symbol_state.hpp"

using namespace std;
using namespace boost::lockfree;

namespace std {
    template <>
    struct hash<pair<size_t, EngineType>> {
        size_t operator()(const pair<size_t, EngineType>& p) const {
            return hash<size_t>()(p.first) ^ (hash<int>()(static_cast<int>(p.second)) << 1);
        }
    };
}

StateOrchestrator::StateOrchestrator() :
    registry_(UniverseRegistry()),
    redis_publisher_(RedisPublisher("localhost", 6379)),
    opened_contracts_(1024)
{}

void StateOrchestrator::initialize_state(vector<Contract>& contracts) {
    registry_.add_contracts(contracts);

    size_t num_symbols = registry_.id_to_symbol().size();
    for (size_t sid = 0; sid < num_symbols; sid++) {
        symbol_table_[sid] = make_unique<SymbolState>(sid);
        
        using expiry_value = pair<vector<PayoffType>, vector<double>>;

        //batch all contracts under the same symbol into engine + expiry batches
        for (size_t eid = 0; eid < registry_.id_to_contract().at(sid).size(); eid++) {
            const auto& expiry_map = registry_.id_to_contract().at(sid).at(eid);
            unordered_map<EngineType, expiry_value> engine_batches;
            for (const ContractDetails& contract : expiry_map) {
                EngineType engine_type = UniverseRegistry::engine_of(contract.payoff_type);
                expiry_value& contracts = engine_batches[engine_type];
                contracts.first.push_back(contract.payoff_type);
                contracts.second.push_back(contract.strike_scaled * 1.0 / STRIKE_SCALE);
            }

            //sort the batches by payoff type and create ExpiryBatch
            for (auto it = engine_batches.begin(); it != engine_batches.end(); it++) {
                EngineType engine_type = it->first;
                vector<PayoffType>& payoff_types = it->second.first;
                vector<double>& strikes = it->second.second;
                t_ns expiry_ns = registry_.id_to_expiry()[sid][eid];

                vector<pair<size_t, size_t>> ranges = build_expiry_batch(payoff_types, strikes);

                switch(engine_type) {
                    case BS_ANALYTIC:
                        symbol_table_[sid]->add_expiry_batch(
                            eid, expiry_ns, engine_type, 
                            std::move(strikes), std::move(payoff_types), std::move(ranges)
                        );
                        break;
                    default:
                        cout << "no valid engine type\n";
                        break;
                }
            }
        }
    }
}

vector<pair<size_t, size_t>> StateOrchestrator::build_expiry_batch(vector<PayoffType>& payoff_types, vector<double>& strikes) {
    unordered_map<int, pair<vector<PayoffType>, vector<double>>> payoff_groups;
    for (size_t i = 0; i < payoff_types.size(); i++) {
        PayoffType payoff = payoff_types[i];
        int payoff_group_id = get_payoff_group_id(payoff);
        payoff_groups[payoff_group_id].first.push_back(payoff);
        payoff_groups[payoff_group_id].second.push_back(strikes[i]);
    }
    
    payoff_types.clear();
    strikes.clear();
    vector<pair<size_t, size_t>> ranges;
    
    for (auto& [payoff_group_id, group] : payoff_groups) {
        size_t start_idx = payoff_types.size();
        
        payoff_types.insert(payoff_types.end(), group.first.begin(), group.first.end());
        strikes.insert(strikes.end(), group.second.begin(), group.second.end());
        
        size_t end_idx = payoff_types.size();
        ranges.emplace_back(start_idx, end_idx);
        
        // Add headroom
        size_t headroom_size = max((size_t)(group.first.size() * BATCH_HEADROOM), (size_t)5);
        for (size_t i = 0; i < headroom_size; i++) {
            payoff_types.push_back(PAYOFFTYPE_ERROR);
            strikes.push_back(0.0);
        }
    }
    
    return std::move(ranges);
}

void StateOrchestrator::process_tick(MarketData& market_data) {
    const auto& symbol_to_id = registry_.symbol_to_id();
    if (symbol_to_id.contains(market_data.symbol)) {
        size_t symbol_id = symbol_to_id.at(market_data.symbol);
        auto& symbol_state = symbol_table_[symbol_id];
        symbol_state->process_tick(market_data);

        build_and_publish_jobs(symbol_state, market_data);
    }
}

void StateOrchestrator::build_and_publish_jobs(unique_ptr<SymbolState>& symbol_state, MarketData& market_data) {
    size_t seqno = symbol_state->seqno();
    size_t calibration_version = symbol_state->calibration_version();

    for (const auto& [expiry_id, expiry_vec] : symbol_state->batches()) {
        for (const auto& batch : expiry_vec) {
            PublishJob job {
                registry_.id_to_symbol().at(symbol_state->symbol_id()),
                market_data.ts_ns,
                market_data.spot, market_data.vol, market_data.rate, market_data.div_yield,
                calibration_version, seqno,
                batch->expiry_ts_ns(),
                batch->tau(),
                batch->engine_type(),
                &batch->strikes(),
                &batch->payoff_types(),
                &batch->ranges(),
                batch->theo(), batch->delta(), batch->gamma(), batch->vega(), batch->rho(), batch->theta()
            };

            redis_publisher_.enqueue_job(std::move(job));
        }
    }
}

void StateOrchestrator::sink_contract_changes(vector<Contract>& contracts) {
    for (Contract& contract : contracts) {
        opened_contracts_.push(std::move(contract));
    }
}

void StateOrchestrator::flush_changes() {
    retire_expiry_slices();
    
    vector<ContractMeta> new_contract_metas;
    registry_.flush_user_changes(opened_contracts_, new_contract_metas);
    
    // Keep track of newly created or recreated batches to avoid duplicate processing
    set<tuple<size_t, size_t, EngineType>> processed_batches;
    
    for (const ContractMeta& contract_meta: new_contract_metas) {
        ensure_symbol_state(contract_meta.symbol_id);
        auto& symbol_state = symbol_table_[contract_meta.symbol_id];
        
        // Create unique identifier for this batch
        auto batch_key = make_tuple(contract_meta.symbol_id, contract_meta.expiry_id, contract_meta.engine_type);
        
        // Check if we need to create a new expiry batch or recreate an existing one
        bool need_new_batch = false;
        string reason;
        check_batch_need(symbol_state, contract_meta, need_new_batch, reason);
        
        if (need_new_batch) {
            // Check if we've already processed this batch in this flush cycle
            if (processed_batches.find(batch_key) != processed_batches.end()) {
                // This batch was already recreated, the contract should already be included
                continue;
            }
            
            // Mark this batch as processed
            processed_batches.insert(batch_key);
            
            // If we're recreating due to headroom, remove the old batch first
            if (reason == "headroom ran out") {
                remove_old_batch(symbol_state, contract_meta);
            }
            
            create_or_recreate_batch(symbol_state, contract_meta);
        } else {
            add_to_existing_batch(symbol_state, contract_meta);
        }
    }
}

void StateOrchestrator::retire_expiry_slices() {
    vector<pair<size_t, size_t>> retired_expiries;
    registry_.find_expired_slices(retired_expiries);
    for (const auto& [sid, eid] : retired_expiries) {
        auto& symbol_state = symbol_table_[sid];
        symbol_state->retire_expiry_slice(eid);
    }
}

void StateOrchestrator::ensure_symbol_state(size_t symbol_id) {
    if (symbol_table_.find(symbol_id) == symbol_table_.end()) {
        symbol_table_[symbol_id] = make_unique<SymbolState>(symbol_id);
    }
}

void StateOrchestrator::check_batch_need(
    unique_ptr<SymbolState>& symbol_state,
    const ContractMeta& contract_meta,
    bool& need_new_batch,
    string& reason
) {
    if (symbol_state->batches().find(contract_meta.expiry_id) == symbol_state->batches().end()) {
        need_new_batch = true;
        reason = "new expiry";
        return;
    }
    
    // Check if engine type exists for this expiry
    auto& expiry_batches = symbol_state->batches().at(contract_meta.expiry_id);
    bool engine_exists = false;
    bool has_headroom = false;
    
    for (auto& batch : expiry_batches) {
        if (batch->engine_type() == contract_meta.engine_type) {
            engine_exists = true;
            
            // Check if any range group has headroom for this payoff type
            const auto& ranges = batch->ranges();
            for (size_t i = 0; i < ranges.size(); i++) {
                pair<size_t, size_t> range = ranges.at(i);
                if (are_payoffs_in_same_group(batch->payoff_types().at(range.first), contract_meta.payoff_type)) {
                    if (batch->has_headroom(i)) {
                        has_headroom = true;
                    }
                    break;
                }
            }
            break;
        }
    }
    
    if (!engine_exists) {
        need_new_batch = true;
        reason = "new engine type";
    } else if (!has_headroom) {
        need_new_batch = true;
        reason = "headroom ran out";
    }
}

void StateOrchestrator::remove_old_batch(
    unique_ptr<SymbolState>& symbol_state,
    const ContractMeta& contract_meta
) {
    auto& expiry_batches = symbol_state->batches().at(contract_meta.expiry_id);
    auto it = remove_if(expiry_batches.begin(), expiry_batches.end(),
        [&contract_meta](const auto& batch) {
            return batch->engine_type() == contract_meta.engine_type;
        });
    expiry_batches.erase(it, expiry_batches.end());
}

void StateOrchestrator::create_or_recreate_batch(
    unique_ptr<SymbolState>& symbol_state,
    const ContractMeta& contract_meta
) {
    // Collect all contracts for this symbol_id + expiry_id + engine_type from registry
    vector<PayoffType> payoff_types;
    vector<double> strikes;
    
    const auto& expiry_contracts = registry_.id_to_contract()
        .at(contract_meta.symbol_id)
        .at(contract_meta.expiry_id);
    
    for (const ContractDetails& contract_details : expiry_contracts) {
        EngineType contract_engine = UniverseRegistry::engine_of(contract_details.payoff_type);
        if (contract_engine == contract_meta.engine_type) {
            payoff_types.push_back(contract_details.payoff_type);
            strikes.push_back(contract_details.strike_scaled * 1.0 / STRIKE_SCALE);
        }
    }
    
    if (!payoff_types.empty()) {
        t_ns expiry_ns = registry_.id_to_expiry()[contract_meta.symbol_id][contract_meta.expiry_id];
        vector<pair<size_t, size_t>> ranges = build_expiry_batch(payoff_types, strikes);
        
        symbol_state->add_expiry_batch(
            contract_meta.expiry_id, expiry_ns, contract_meta.engine_type,
            std::move(strikes), std::move(payoff_types), std::move(ranges)
        );
    }
}

void StateOrchestrator::add_to_existing_batch(
    unique_ptr<SymbolState>& symbol_state,
    const ContractMeta& contract_meta
) {
    auto& affected_slice = symbol_state->batches().at(contract_meta.expiry_id);

    for (auto& batch : affected_slice) {
        if (batch->engine_type() == contract_meta.engine_type) {
            const auto& ranges = batch->ranges();
            for (size_t i = 0; i < ranges.size(); i++) {
                pair<size_t, size_t> range = batch->ranges().at(i);
                if (are_payoffs_in_same_group(batch->payoff_types().at(range.first), contract_meta.payoff_type)) {
                    if (batch->has_headroom(i)) {
                        const ContractDetails& contract_details = (
                            registry_.id_to_contract()
                            .at(contract_meta.symbol_id)
                            .at(contract_meta.expiry_id)
                            .at(contract_meta.contract_id)
                        );
                        batch->append_new_contract((contract_details.strike_scaled * 1.0 / STRIKE_SCALE), contract_details.payoff_type, i);
                    }
                    break; 
                }
            }
            break;
        }
    }
}