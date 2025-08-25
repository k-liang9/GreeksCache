#include <iostream>
#include <vector>
#include <iomanip>
#include "bs_engine.hpp"
#include "testing_vars.hpp"
#include "options.hpp"
#include "registry.hpp"
#include "state_orchestrator.hpp"
#include "utils.hpp"
#include "gbm_sim.hpp"

using namespace std;

int main() {
    cout << "=== Greeks Engine Verification Test ===" << endl;
    cout << endl;
    
    UniverseRegistry registry = UniverseRegistry(test::contracts);
    StateOrchestrator orchestrator = StateOrchestrator(registry);
    orchestrator.initialize_state();
    
    // Print contract details organized by expiry batch
    cout << "=== CONTRACT LISTING ===" << endl;
    cout << "Total Contracts: " << test::contracts.size() << endl << endl;
    
    size_t contract_num = 0;
    for (const auto& [symbol_id, symbol_state] : orchestrator.symbol_table()) {
        size_t batch_num = 0;
        for (auto& batch : symbol_state->batches()) {
            double time_to_expiry = ns_to_yrs(batch->expiry_ts_ns() - now());
            cout << "Expiry Batch " << batch_num++ << " (T=" << fixed << setprecision(4) << time_to_expiry << " years):" << endl;
            
            const auto& strikes = batch->strikes();
            const auto& payoff_types = batch->payoff_types();
            
            for (size_t i = 0; i < strikes.size(); i++) {
                cout << "  Contract " << contract_num++ << ": " 
                     << test::contracts[contract_num-1].symbol << " "
                     << test::contracts[contract_num-1].expiry << " "
                     << "$" << setw(6) << strikes[i] << " " 
                     << (payoff_types[i] == VAN_CALL ? "CALL" : "PUT ") << endl;
            }
            cout << endl;
        }
    }
    cout << endl;
    
    GbmSimulator sim;
    sim.input_sim_data(
        test::market_conditions.start_ts,
        test::market_conditions.dt,
        test::market_conditions.sim_duration,
        test::market_conditions.S0,
        test::market_conditions.vol,
        test::market_conditions.rate,
        test::market_conditions.div_yield,
        test::market_conditions.drift,
        test::market_conditions.symbol
    );
    vector<MarketData> sim_data;
    sim.generate_sim_data(sim_data);

    for (auto& data : sim_data) {
        orchestrator.process_tick(data);

        cout << "=== TICK: " << data.symbol << " Spot=$" << fixed << setprecision(2) << data.spot 
             << " Time=" << ns_to_s(data.ts_ns) << "s ===" << endl;
        
        for (const auto& [symbol_id, symbol_state] : orchestrator.symbol_table()) {
            size_t contract_num = 0;
            for (auto& batch : symbol_state->batches()) {
                const auto& strikes = batch->strikes();
                const auto& payoff_types = batch->payoff_types();
                const auto& theos = batch->theo();
                const auto& deltas = batch->delta();
                const auto& gammas = batch->gamma();
                const auto& vegas = batch->vega();
                const auto& rhos = batch->rho();
                const auto& thetas = batch->theta();
                
                for (size_t i = 0; i < strikes.size(); i++) {
                    cout << "  Option " << contract_num++ << ": " 
                         << "$" << setw(7) << fixed << setprecision(2) << strikes[i] << " " 
                         << (payoff_types[i] == VAN_CALL ? "CALL" : "PUT ") << endl
                         << "    Theo=$" << setw(8) << setprecision(4) << theos[i]
                         << " | Δ=" << setw(8) << setprecision(6) << deltas[i]
                         << " | Γ=" << setw(8) << setprecision(6) << gammas[i] << endl
                         << "    Vega=" << setw(8) << setprecision(6) << vegas[i]
                         << " | ρ=" << setw(8) << setprecision(6) << rhos[i]
                         << " | Θ=" << setw(8) << setprecision(6) << thetas[i] << endl;
                }
            }
        }
        cout << endl;
    }

    return 0;
}