#include <iostream>
#include <vector>
#include "bs_engine.hpp"
#include "testing_vars.hpp"
#include "options.hpp"
#include "registry.hpp"
#include "state_orchestrator.hpp"
#include "utils.hpp"

using namespace std;

int main() {
    UniverseRegistry registry = UniverseRegistry(test::contracts);
    StateOrchestrator orchestrator = StateOrchestrator();
    orchestrator.initialize_state(registry);
    for (auto& [symbol_id, symbol_state] : orchestrator.symbol_table) {
        for (auto& batch : symbol_state->batches()) {
            cout << "expiry (yrs):" << ns_to_yrs(batch->expiry_ts_ns() - now()) << '\n';
            for (size_t i = 0; i < batch->strikes().size(); i++) {
                cout << "  strike: " << batch->strikes()[i] << " payoff type: " << batch->payoff_types()[i] << "\n";
            }
        }
    }
    return 0;
}