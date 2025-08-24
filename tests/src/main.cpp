#include <iostream>
#include <vector>
#include <iomanip>
#include "bs_engine.hpp"
#include "testing_vars.hpp"
#include "options.hpp"
#include "registry.hpp"
#include "state_orchestrator.hpp"
#include "utils.hpp"

using namespace std;

int main() {
    cout << "=== Greeks Engine Verification Test ===" << endl;
    cout << endl;
    
    UniverseRegistry registry = UniverseRegistry(test::contracts);
    StateOrchestrator orchestrator = StateOrchestrator(registry);
    orchestrator.initialize_state();
    
    cout << "Market Data:" << endl;
    cout << "  Symbol: " << test::market_data.symbol << endl;
    cout << "  Spot Price: $" << test::market_data.spot << endl;
    cout << "  Volatility: " << (test::market_data.vol * 100) << "%" << endl;
    cout << "  Risk-free Rate: " << (test::market_data.rate * 100) << "%" << endl;
    cout << "  Dividend Yield: " << (test::market_data.div_yield * 100) << "%" << endl;
    cout << endl;
    
    cout << "Contracts Before Processing:" << endl;
    for (const auto& [symbol_id, symbol_state] : orchestrator.symbol_table()) {
        for (auto& batch : symbol_state->batches()) {
            double time_to_expiry = ns_to_yrs(batch->expiry_ts_ns() - now());
            cout << "  Expiry Batch (Time to expiry: " << time_to_expiry << " years):" << endl;
            for (size_t i = 0; i < batch->strikes().size(); i++) {
                cout << "    Contract " << i << ": Strike=$" << batch->strikes()[i] 
                     << ", Type=" << (batch->payoff_types()[i] == VAN_CALL ? "CALL" : "PUT") << endl;
            }
        }
    }
    cout << endl;

    cout << "Processing market tick..." << endl;
    cout << "Market data symbol: " << test::market_data.symbol << endl;
    
    try {
        // Check if the symbol exists in the registry
        const auto& symbol_to_id = registry.get_symbol_to_id();
        if (symbol_to_id.find(test::market_data.symbol) == symbol_to_id.end()) {
            cout << "Error: Symbol '" << test::market_data.symbol << "' not found in registry!" << endl;
            return 1;
        }
        
        cout << "Symbol found in registry, processing..." << endl;
        orchestrator.process_tick(test::market_data);
        cout << "Market tick processed successfully!" << endl;
    } catch (const exception& e) {
        cout << "Error processing market tick: " << e.what() << endl;
        return 1;
    } catch (...) {
        cout << "Unknown error processing market tick!" << endl;
        return 1;
    }
    cout << endl;
    
    cout << "=== GREEKS RESULTS ===" << endl;
    for (const auto& [symbol_id, symbol_state] : orchestrator.symbol_table()) {
        for (auto& batch : symbol_state->batches()) {
            double time_to_expiry = ns_to_yrs(batch->expiry_ts_ns() - now());
            cout << "Expiry Batch (T=" << fixed << setprecision(4) << time_to_expiry << " years):" << endl;
            cout << "  Contract | Strike  | Type | Theo     | Delta   | Gamma   | Vega    | Rho     | Theta   " << endl;
            cout << "  ---------|---------|------|----------|---------|---------|---------|---------|--------" << endl;
            
            const auto& strikes = batch->strikes();
            const auto& payoff_types = batch->payoff_types();
            const auto& theos = batch->theo();
            const auto& deltas = batch->delta();
            const auto& gammas = batch->gamma();
            const auto& vegas = batch->vega();
            const auto& rhos = batch->rho();
            const auto& thetas = batch->theta();
            
            for (size_t i = 0; i < strikes.size(); i++) {
                cout << "  " << setw(8) << i 
                     << " | " << setw(7) << fixed << setprecision(2) << strikes[i]
                     << " | " << setw(4) << (payoff_types[i] == VAN_CALL ? "CALL" : "PUT")
                     << " | " << setw(8) << fixed << setprecision(4) << theos[i]
                     << " | " << setw(7) << fixed << setprecision(4) << deltas[i]
                     << " | " << setw(7) << fixed << setprecision(4) << gammas[i]
                     << " | " << setw(7) << fixed << setprecision(4) << vegas[i]
                     << " | " << setw(7) << fixed << setprecision(4) << rhos[i]
                     << " | " << setw(7) << fixed << setprecision(4) << thetas[i]
                     << endl;
            }
            cout << endl;
        }
    }
    
    cout << "=== Verification Summary ===" << endl;
    cout << "✓ Registry initialized with " << test::contracts.size() << " contracts" << endl;
    cout << "✓ State orchestrator initialized" << endl;
    cout << "✓ Market tick processed" << endl;
    cout << "✓ Greeks calculated for all contracts" << endl;
    cout << endl;
    cout << "Expected behaviors to verify:" << endl;
    cout << "- Call deltas should be positive (0 < δ < 1)" << endl;
    cout << "- Put deltas should be negative (-1 < δ < 0)" << endl;
    cout << "- Gamma should be positive for both calls and puts" << endl;
    cout << "- Vega should be positive for both calls and puts" << endl;
    cout << "- Theta should typically be negative (time decay)" << endl;
    cout << "- ITM options should have higher absolute deltas" << endl;
    cout << "- ATM options should have highest gamma and vega" << endl;

    return 0;
}