#ifndef STATE_ORCHESTRATOR
#define STATE_ORCHESTRATOR

#include <vector>
#include <unordered_map>
#include <functional>
#include "symbol_state.hpp"
#include "registry.hpp"

using namespace std;
class SymbolState;

class StateOrchestrator {
public:
    StateOrchestrator() {}

    std::unordered_map<int, SymbolState*> symbol_table;
    void build_symbol_table(const UniverseRegistry& registry);
    void create_symbol_state(int symbol_id);
    void replace_symbol_state(int symbol_id, SymbolState* new_state);
    void destroy_symbol_state(int symbol_id);
    void drain_contract_changes();
    void run_tick_schedule();

private:
};

#endif