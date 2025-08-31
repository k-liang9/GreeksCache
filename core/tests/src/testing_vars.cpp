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
        {"AAPL", ns_to_iso8601_ny(now_ns + s_to_ns(5*86400), false), 105.0, VAN_CALL},
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
        {"GOOGL", ns_to_iso8601_ny(now_ns + s_to_ns(10*86400), false), 250.4, VAN_CALL}
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
}
