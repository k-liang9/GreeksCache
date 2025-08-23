#ifndef STATE_ORCHESTRATOR
#define STATE_ORCHESTRATOR

#include <vector>
#include <unordered_map>
#include <functional>
#include "symbol_state.hpp"
#include "registry.hpp"
#include <memory>

using namespace std;

class SymbolState;

class StateOrchestrator {
public:
    StateOrchestrator() {}

    std::unordered_map<int, unique_ptr<SymbolState>> symbol_table;
    void initialize_state(const UniverseRegistry& registry);

private:
};

#endif