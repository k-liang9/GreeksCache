#include <vector>
#include <unordered_map>
#include <algorithm>
#include <memory>
#include <numeric>
#include "state_orchestrator.hpp"
#include "registry.hpp"
#include "expiry_state.hpp"
#include "symbol_state.hpp"

using namespace std;

void StateOrchestrator::initialize_state(const UniverseRegistry& registry) {
    size_t num_symbols = registry.id_to_symbol().size();
    for (size_t sid = 0; sid < num_symbols; sid++) {
        symbol_table[sid] = make_unique<SymbolState>(sid);
        
        using expiry_key = pair<size_t, EngineType>;
        using expiry_value = pair<vector<PayoffType>, vector<double>>;

        //batch all contracts under the same symbol into engine + expiry batches
        unordered_map<expiry_key, expiry_value> expiry_batches;
        for (const contract_t& contract : registry.id_to_contract()[sid]) {
            EngineType engine_type = UniverseRegistry::engine_of(get<2>(contract));
            expiry_key key = {get<0>(contract), engine_type};
            expiry_value& contracts = expiry_batches[key];
            contracts.first.push_back(get<2>(contract));
            contracts.second.push_back(get<1>(contract));
        }
        
        //sort the batches by payoff type and create ExpiryBatch
        for (auto it = expiry_batches.begin(); it != expiry_batches.end(); it++) {
            expiry_key key = it->first;
            vector<PayoffType>& payoff_types = it->second.first;
            vector<double>& strikes = it->second.second;

            // Create index vector for sorting
            vector<size_t> indices(payoff_types.size());
            iota(indices.begin(), indices.end(), 0);
            
            // Sort indices based on PayoffType values
            sort(indices.begin(), indices.end(), 
                [&payoff_types](size_t a, size_t b) {
                    return payoff_types[a] < payoff_types[b];
                });
            
            // Rearrange both vectors according to sorted indices
            vector<PayoffType> sorted_payoffs(payoff_types.size());
            vector<double> sorted_strikes(strikes.size());
            
            for (size_t i = 0; i < indices.size(); i++) {
                sorted_payoffs[i] = payoff_types[indices[i]];
                sorted_strikes[i] = strikes[indices[i]];
            }
            
            payoff_types = move(sorted_payoffs);
            strikes = move(sorted_strikes);

            switch(key.second) {
                case BS_ANALYTIC:
                    symbol_table[sid]->batches.emplace_back(
                        make_unique<BSBatch>(key.first, key.second, move(strikes), move(payoff_types))
                    );
                    break;
                default:
                    break;
            }
        }
    }
}