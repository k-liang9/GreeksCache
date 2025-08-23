#include <vector>
#include <string>
#include <array>
#include <unordered_map>
#include "registry.hpp"
#include "utils.hpp"
#include "options.hpp"

using namespace std;

UniverseRegistry::UniverseRegistry(vector<Contract>& contracts) {
    add_contracts(contracts);
}

void UniverseRegistry::add_contracts(vector<Contract>& contracts) {
    for (Contract& contract : contracts) {
        if (!symbol_to_id.contains(contract.symbol)) {
            id_to_symbol.push_back(contract.symbol);
            symbol_to_id[contract.symbol] = id_to_symbol.size() - 1;
            expiry_to_id.emplace_back(unordered_map<t_ns, size_t>{});
            id_to_expiry.emplace_back(vector<t_ns>{});
            contract_to_id.emplace_back(unordered_map<ContractKey, size_t, ContractKeyHash>{});
            id_to_contract.emplace_back(vector<ContractKey>{});
        }

        size_t symbol_id = symbol_to_id[contract.symbol];
        t_ns expiry_ns = parse_time(contract.expiry);

        if (!expiry_to_id[symbol_id].contains(expiry_ns)) {
            id_to_expiry[symbol_id].push_back(expiry_ns);
            expiry_to_id[symbol_id][expiry_ns] = id_to_expiry[symbol_id].size() - 1;
        }

        size_t expiry_id = expiry_to_id[symbol_id][expiry_ns];
        
        ContractKey contract_key = {expiry_id, contract.strike, contract.payoff_type};
        if (!contract_to_id[symbol_id].contains(contract_key)) {
            id_to_contract[symbol_id].push_back(contract_key);
            contract_to_id[symbol_id][contract_key] = id_to_contract[symbol_id].size() - 1;
        }
    }
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