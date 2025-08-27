#include <vector>
#include <unordered_map>
#include <algorithm>
#include <memory>
#include <iostream>
#include <numeric>
#include "state_orchestrator.hpp"
#include "registry.hpp"
#include "expiry_state.hpp"
#include "symbol_state.hpp"

using namespace std;

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
    redis_publisher_(RedisPublisher("localhost", 6379))
{}

void StateOrchestrator::initialize_state(vector<Contract>& contracts) {
    registry_.add_contracts(contracts);

    size_t num_symbols = registry_.get_id_to_symbol().size();
    for (size_t sid = 0; sid < num_symbols; sid++) {
        symbol_table_[sid] = make_unique<SymbolState>(sid);
        
        using expiry_key = pair<size_t, EngineType>;
        using expiry_value = pair<vector<PayoffType>, vector<double>>;

        //batch all contracts under the same symbol into engine + expiry batches
        unordered_map<expiry_key, expiry_value> expiry_batches;
        for (const ContractKey& contract : registry_.get_id_to_contract()[sid]) {
            EngineType engine_type = UniverseRegistry::engine_of(contract.payoff_type);
            expiry_key key = {contract.expiry_id, engine_type};
            expiry_value& contracts = expiry_batches[key];
            contracts.first.push_back(contract.payoff_type);
            contracts.second.push_back(contract.strike);
        }
        
        //sort the batches by payoff type and create ExpiryBatch
        for (auto it = expiry_batches.begin(); it != expiry_batches.end(); it++) {
            size_t expiry_id = it->first.first;
            EngineType engine_type = it->first.second;
            vector<PayoffType>& payoff_types = it->second.first;
            vector<double>& strikes = it->second.second;
            t_ns expiry_ns = registry_.get_id_to_expiry()[sid][expiry_id];

            vector<pair<size_t, size_t>> ranges = build_expiry_batch(payoff_types, strikes);

            switch(engine_type) {
                case BS_ANALYTIC:
                    symbol_table_[sid]->add_expiry_batch(
                        expiry_id, expiry_ns, engine_type, 
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

vector<pair<size_t, size_t>> StateOrchestrator::build_expiry_batch(vector<PayoffType>& payoff_types, vector<double>& strikes) {
    unordered_map<PayoffType, pair<vector<PayoffType>, vector<double>>> payoff_groups;
    for (size_t i = 0; i < payoff_types.size(); i++) {
        PayoffType payoff = payoff_types[i];
        PayoffType payoff_group;
        switch (payoff) {
        case VAN_CALL:
        case VAN_PUT:
            payoff_group = VANILLA;
        default:
            payoff_group = payoff;
        }
        payoff_groups[payoff_group].first.push_back(payoff);
        payoff_groups[payoff_group].second.push_back(strikes[i]);
    }
    
    payoff_types.clear();
    strikes.clear();
    vector<pair<size_t, size_t>> ranges;
    
    for (auto& [payoff, group] : payoff_groups) {
        size_t start_idx = payoff_types.size();
        
        payoff_types.insert(payoff_types.end(), group.first.begin(), group.first.end());
        strikes.insert(strikes.end(), group.second.begin(), group.second.end());
        
        size_t end_idx = payoff_types.size();
        ranges.emplace_back(start_idx, end_idx);
        
        // Add headroom
        size_t headroom_size = group.first.size() * BATCH_HEADROOM;
        for (size_t i = 0; i < headroom_size; i++) {
            payoff_types.push_back(payoff);
            strikes.push_back(0.0);
        }
    }
    
    return std::move(ranges);
}

void StateOrchestrator::process_tick(MarketData& market_data) {
    const auto& symbol_to_id = registry_.get_symbol_to_id();
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

    for (auto& batch : symbol_state->batches()) {
        PublishJob job {
            registry_.get_id_to_symbol().at(symbol_state->symbol_id()),
            market_data.ts_ns,
            market_data.spot, market_data.vol, market_data.rate, market_data.div_yield,
            calibration_version, seqno,
            batch->expiry_ts_ns(),
            batch->engine_type(),
            &batch->strikes(),
            &batch->payoff_types(),
            &batch->ranges(),
            batch->theo(), batch->delta(), batch->gamma(), batch->vega(), batch->rho(), batch->theta()
        };

        redis_publisher_.enqueue_job(std::move(job));
    }
}

void StateOrchestrator::add_contracts(vector<Contract>& contracts) {}