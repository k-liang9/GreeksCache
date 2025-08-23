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
            expiry_to_id.emplace_back(unordered_map<string, size_t>{});
            id_to_expiry.emplace_back(vector<string>{});
            contract_to_id.emplace_back(unordered_map<string, size_t>{});
            id_to_contract.emplace_back(vector<string>{});
        }

        size_t symbol_id = symbol_to_id[contract.symbol];

        if (!expiry_to_id[symbol_id].contains(contract.expiry)) {
            id_to_expiry[symbol_id].push_back(contract.expiry);
            expiry_to_id[symbol_id][contract.expiry] = id_to_expiry[symbol_id].size() - 1;
        }

        size_t expiry_id = expiry_to_id[symbol_id][contract.expiry];
        
        string contract_key;
        contract_to_string(contract_key, expiry_id, contract.strike, contract.payoff_type);
        if (!contract_to_id[symbol_id].contains(contract_key)) {
            id_to_contract[symbol_id].push_back(contract_key);
            contract_to_id[symbol_id][contract_key] = id_to_contract[symbol_id].size() - 1;
        }

        size_t contract_id = contract_to_id[symbol_id][contract_key];
    }
}

void UniverseRegistry::contract_to_string(
    string& contract, 
    const size_t expiry_id, 
    const double strike, 
    const PayoffType payoff_type) {
    contract = to_string(expiry_id) + ":" + to_string(strike) + ":" + to_string(static_cast<int>(payoff_type));
}

EngineType UniverseRegistry::engine_of(PayoffType type) {
    switch(type) {
        case VAN_CALL or VAN_PUT:
            return BS_ANALYTIC;
            break;
        default:
            return ENGINETYPE_ERROR;
            break;
    }
}