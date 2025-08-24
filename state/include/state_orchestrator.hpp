#ifndef STATE_ORCHESTRATOR
#define STATE_ORCHESTRATOR

#define BATCH_HEADROOM 0.2;

#include <vector>
#include <unordered_map>
#include <functional>
#include "symbol_state.hpp"
#include "registry.hpp"
#include <memory>
#include <queue>
#include "market_data.hpp"

using namespace std;

class SymbolState;

class StateOrchestrator {
public:
    StateOrchestrator(const UniverseRegistry& registry);

    void initialize_state();
    vector<pair<size_t, size_t>> build_expiry_batch(vector<PayoffType>& payoff_types, vector<double>& strikes);
    void process_tick(MarketData& market_data);
    const unordered_map<size_t, unique_ptr<SymbolState>>& symbol_table() const { return symbol_table_; }

private:
    unordered_map<size_t, unique_ptr<SymbolState>> symbol_table_;
    const UniverseRegistry& registry_;
    queue<Contract> changes;
};

#endif