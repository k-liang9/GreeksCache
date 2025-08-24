#include <iostream>
#include <vector>
#include "bs_engine.hpp"
#include "testing_vars.hpp"
#include "options.hpp"
#include "registry.hpp"
#include "state_orchestrator.hpp"

using namespace std;

int main() {
    UniverseRegistry registry = UniverseRegistry(test::contracts);
    StateOrchestrator orchestrator = StateOrchestrator();
    orchestrator.initialize_state(registry);
    for (auto& [symbol_id, symbol_state] : orchestrator.symbol_table) {
        cout << symbol_id << ":\n";
        for (auto& batch : symbol_state->batches()) {
            cout << '\t' << batch->expiry_id() << '\n';
            cout << '\t' << batch->expiry_ts_ns() << '\n';
            auto engineTypeToString = [](EngineType type) {
                switch (type) {
                    case BS_ANALYTIC: return "BS";
                    default: return "UNKNOWN";
                }
            };
            cout << '\t' << engineTypeToString(batch->engine_type()) << '\n';
        }
    }

    // const array<const char*, NUM_OPTION_TYPES> type_names = {"CALL", "PUT"};
    // for (int type = 0; type < NUM_OPTION_TYPES; ++type) {
    //     const auto& prices = AAPL_contracts.prices_[type];
    //     const auto& deltas = AAPL_contracts.deltas_[type];
    //     const auto& gammas = AAPL_contracts.gammas_[type];
    //     const auto& vegas  = AAPL_contracts.vegas_[type];
    //     const auto& rhos   = AAPL_contracts.rhos_[type];
    //     const auto& thetas = AAPL_contracts.thetas_[type];
    //     cout << type_names[type] << " contracts:" << endl;
    //     for (size_t i = 0; i < prices.size(); ++i) {
    //         cout << "  Contract " << i + 1 << ":" << endl;
    //         cout << "    Price=" << prices[i] << endl;
    //         cout << "    Delta=" << deltas[i] << endl;
    //         cout << "    Gamma=" << gammas[i] << endl;
    //         cout << "    Vega=" << vegas[i] << endl;
    //         cout << "    Rho=" << rhos[i] << endl;
    //         cout << "    Theta=" << thetas[i] << endl;
    //     }
    //     cout << endl;
    // }
    return 0;
}