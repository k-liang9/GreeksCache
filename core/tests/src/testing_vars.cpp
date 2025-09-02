#include <string>
#include "testing_vars.hpp"
#include "utils.hpp"
#include "types.hpp"

using namespace std;

namespace test {
    t_ns now_ns = now();
    string test_expiry = ns_to_iso8601_ny(now_ns + s_to_ns(86400), false);
    vector<Contract> contracts = {
        {"AAPL", ns_to_iso8601_ny(now_ns + s_to_ns(5*86400), false), 100.0, VAN_CALL},
        {"AAPL", ns_to_iso8601_ny(now_ns + s_to_ns(5*86400), false), 105.0, VAN_PUT},
        {"AAPL", ns_to_iso8601_ny(now_ns + s_to_ns(90*86400), false), 110.0, VAN_CALL},
        {"AAPL", ns_to_iso8601_ny(now_ns + s_to_ns(120*86400), false), 95.0, VAN_PUT},
        {"AAPL", ns_to_iso8601_ny(now_ns + s_to_ns(60*86400), false), 120.0, VAN_CALL},
        {"AAPL", ns_to_iso8601_ny(now_ns + s_to_ns(30*86400), false), 104.4, VAN_PUT},
        {"AAPL", ns_to_iso8601_ny(now_ns + s_to_ns(3*86400), false), 90.0, VAN_PUT},
        {"GOOGL", ns_to_iso8601_ny(now_ns + s_to_ns(10*86400), false), 250.4, VAN_CALL},
        {"GOOGL", ns_to_iso8601_ny(now_ns + s_to_ns(86400), false), 251, VAN_PUT},
        {"GOOGL", ns_to_iso8601_ny(now_ns + s_to_ns(15), true), 50, VAN_CALL}
    };
    vector<Contract> user_changes = {
        // Test case 1: New symbol (TSLA) - should create new SymbolState
        {"TSLA", ns_to_iso8601_ny(now_ns + s_to_ns(7*86400), false), 200.0, VAN_CALL},
        
        // Test case 2: Existing symbol (GOOGL) with new expiry - should create new expiry batch
        {"GOOGL", ns_to_iso8601_ny(now_ns + s_to_ns(20*86400), false), 260.0, VAN_PUT},
        
        // Test case 3: Existing symbol and expiry but should fit in headroom
        {"GOOGL", ns_to_iso8601_ny(now_ns + s_to_ns(10*86400), false), 255.0, VAN_CALL},
        
        // Test case 4: Multiple contracts to test headroom exhaustion
        // Adding many contracts to the same AAPL 5-day expiry to potentially exhaust headroom
        {"AAPL", ns_to_iso8601_ny(now_ns + s_to_ns(5*86400), false), 101.0, VAN_CALL},
        {"AAPL", ns_to_iso8601_ny(now_ns + s_to_ns(5*86400), false), 102.0, VAN_CALL},
        {"AAPL", ns_to_iso8601_ny(now_ns + s_to_ns(5*86400), false), 103.0, VAN_CALL},
        {"AAPL", ns_to_iso8601_ny(now_ns + s_to_ns(5*86400), false), 106.0, VAN_PUT},
        {"AAPL", ns_to_iso8601_ny(now_ns + s_to_ns(5*86400), false), 107.0, VAN_PUT},
        {"AAPL", ns_to_iso8601_ny(now_ns + s_to_ns(5*86400), false), 108.0, VAN_PUT},
        
        // Test case 5: Another new symbol to test multiple new symbols
        {"MSFT", ns_to_iso8601_ny(now_ns + s_to_ns(14*86400), false), 300.0, VAN_PUT},
        
        // Test case 6: Existing symbol (AAPL) with another new expiry
        {"AAPL", ns_to_iso8601_ny(now_ns + s_to_ns(45*86400), false), 115.0, VAN_CALL}
    };
    SimInput apple_market_conditions = {
        now_ns, 10,  // start_ts, dt,
        100.0, 0.20, 0.05, 0.03,           // S0, vol, rate, div_yield
        0.80,                              // drift
        "AAPL"
    };
    SimInput google_market_conditions = {
        now_ns, 15,
        249.5, 0.40, 0.05, 0.06,
        0.90,
        "GOOGL"
    };
    SimInput tsla_market_conditions = {
        now_ns, 20,
        200.0, 0.35, 0.05, 0.02,
        0.85,
        "TSLA"
    };
    SimInput msft_market_conditions = {
        now_ns, 12,
        300.0, 0.25, 0.05, 0.04,
        0.75,
        "MSFT"
    };
}
