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

using namespace std;

class SymbolState;

class StateOrchestrator {
public:
    StateOrchestrator() {}

    std::unordered_map<int, unique_ptr<SymbolState>> symbol_table;
    void initialize_state(const UniverseRegistry& registry);
    void sink_changes();
    vector<pair<size_t, size_t>> build_expiry_batch(vector<PayoffType>& payoff_types, vector<double>& strikes);

private:
    queue<Contract> changes;
};

#endif