#include <iostream>
#include <vector>
#include <iomanip>
#include <atomic>
#include <thread>
#include <unordered_map>
#include <iterator>
#include <sw/redis++/redis++.h>
#include "bs_engine.hpp"
#include "testing_vars.hpp"
#include "types.hpp"
#include "registry.hpp"
#include "state_orchestrator.hpp"
#include "utils.hpp"
#include "gbm_sim.hpp"

using namespace std;
using namespace sw::redis;

void redis_test() {
    auto redis = Redis("tcp://localhost:6379");
    redis.set("kevin",  "liang");
    auto val = redis.get("kevin");
    if (val) {
        cout << *val << '\n';
    }
}

void print_contract_listing(const StateOrchestrator& orchestrator) {
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
}

void print_greeks_tick(const MarketData& data, const StateOrchestrator& orchestrator) {
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

vector<MarketData> generate_simulation_data() {
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
    return sim_data;
}

int main() {
    StateOrchestrator orchestrator = StateOrchestrator();
    orchestrator.initialize_state(test::contracts);
    vector<MarketData> sim_data = generate_simulation_data();
    // print_contract_listing(orchestrator);

    atomic<bool> stop{false};

    thread compute_core([&]{
        for (auto& data : sim_data) {
            orchestrator.process_tick(data);
            // print_greeks_tick(data, orchestrator);
            this_thread::sleep_for(chrono::milliseconds(100));
        }
        stop.store(true);
    });

    thread publisher([&]{
        while (!stop.load()) {
            orchestrator.redis_publisher().run();
        }
    });

    thread reader([&]{
        auto redis = Redis("tcp://localhost:6379");
        while (!stop.load()) {
            try {
                unordered_map<string, string> result;
                redis.hgetall("greeks:AAPL:2026-08-20:100.0000:VAN_CALL", inserter(result, result.begin()));
                if (!result.empty()) {
                    cout << "=== REDIS HGETALL RESULT ===" << endl;
                    for (const auto& pair : result) {
                        cout << pair.first << ": " << pair.second << endl;
                    }
                    cout << endl;
                }
            } catch (const exception& e) {
                // Redis connection might not be ready yet, continue silently
            }
            this_thread::sleep_for(chrono::milliseconds(200));
        }
    });
    
    compute_core.join();
    publisher.join();
    reader.join();

    return 0;
}