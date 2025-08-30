#include <vector>
#include <string>
#include <array>
#include <unordered_map>
#include <boost/lockfree/spsc_queue.hpp>
#include "registry.hpp"
#include "utils.hpp"

using namespace std;
using namespace boost::lockfree;

UniverseRegistry::UniverseRegistry(vector<Contract>& contracts) {
    add_contracts(contracts);
    epoch_ = 0;
}

void UniverseRegistry::add_contracts(vector<Contract>& contracts) {
    for (Contract& contract : contracts) {
        MissingLevel missing = missing_level_of(contract);
        if (missing == EXISTS) {
            continue;
        }
        if (missing <= SYMBOL) {
            track_new_symbol(contract.symbol);
        }

        size_t symbol_id = symbol_to_id_[contract.symbol];
        t_ns expiry_ns = parse_time(contract.expiry);

        if (missing <= EXPIRY) {
            add_new_expiry(symbol_id, expiry_ns);
        }

        size_t expiry_id = expiry_to_id_[symbol_id][expiry_ns];

        if (missing <= CONTRACT) {
            add_new_contract(symbol_id, expiry_id, contract.strike, contract.payoff_type);
        }
    }
}

void UniverseRegistry::flush_changes(spsc_queue<Contract>& open, spsc_queue<Contract>& close) {
    Contract contract;
    while (open.pop(contract)) {
        MissingLevel missing = missing_level_of(contract);
        if (missing == EXISTS) {
            continue;
        }
        if (missing <= SYMBOL) {
            track_new_symbol(contract.symbol);
        }

        size_t symbol_id = symbol_to_id_[contract.symbol];
        t_ns expiry_ns = parse_time(contract.expiry);

        if (missing <= EXPIRY) {
            add_new_expiry(symbol_id, expiry_ns);
        }

        size_t expiry_id = expiry_to_id_[symbol_id][expiry_ns];

        if (missing <= CONTRACT) {
            add_new_contract(symbol_id, expiry_id, contract.strike, contract.payoff_type);
        }
    }
    while (close.pop(contract)) {
        MissingLevel missing = missing_level_of(contract);

    }
}


MissingLevel UniverseRegistry::missing_level_of(Contract& contract) {
    if (!symbol_to_id_.contains(contract.symbol)) {
        return SYMBOL;
    }
    size_t symbol_id = symbol_to_id_[contract.symbol];
    t_ns expiry = parse_time(contract.expiry);
    if (!expiry_to_id_[symbol_id].contains(expiry)) {
        return EXPIRY;
    }
    size_t expiry_id = expiry_to_id_[symbol_id][expiry];
    //TODO: will need to fix once expiry-batched contract_ids are implemented
    ContractKey key = {expiry_id, contract.strike, contract.payoff_type};
    if (!contract_to_id_[symbol_id].contains(key)) {
        return CONTRACT;
    } else {
        return EXISTS;
    }
}



void UniverseRegistry::track_new_symbol(string& symbol) {
    id_to_symbol_.push_back(symbol);
    symbol_to_id_[symbol] = id_to_symbol_.size() - 1;
    expiry_to_id_.emplace_back(unordered_map<t_ns, size_t>{});
    id_to_expiry_.emplace_back(vector<t_ns>{});
    contract_to_id_.emplace_back(unordered_map<ContractKey, size_t, ContractKeyHash>{});
    id_to_contract_.emplace_back(vector<ContractKey>{});
}

void UniverseRegistry::add_new_expiry(size_t symbol_id, t_ns expiry_ns) {
    id_to_expiry_[symbol_id].push_back(expiry_ns);
    expiry_to_id_[symbol_id][expiry_ns] = id_to_expiry_[symbol_id].size() - 1;
}

void UniverseRegistry::add_new_contract(size_t symbol_id, size_t expiry_id, float strike, PayoffType payoff_type) {
    ContractKey contract_key = {expiry_id, strike, payoff_type};
    id_to_contract_[symbol_id].push_back(contract_key);
    contract_to_id_[symbol_id][contract_key] = id_to_contract_[symbol_id].size() - 1;
}

EngineType UniverseRegistry::engine_of(PayoffType type) {
    switch(type) {
        case VAN_CALL:
        case VAN_PUT:
            return BS_ANALYTIC;
        default:
            return ENGINETYPE_ERROR;
    }
}