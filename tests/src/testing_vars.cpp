#include "testing_vars.hpp"
#include "utils.hpp"
#include "types.hpp"

using namespace std;

namespace test {
    vector<Contract> contracts = {
        {"AAPL", "2026-08-20", 100.0, VAN_CALL},
        {"AAPL", "2026-09-20", 105.0, VAN_CALL},
        {"AAPL", "2026-10-20", 110.0, VAN_CALL},
        {"AAPL", "2026-11-20", 95.0, VAN_PUT},
        {"AAPL", "2026-12-20", 120.0, VAN_CALL},
        {"AAPL", "2026-08-20", 104.4, VAN_PUT},
        {"AAPL", "2027-01-20", 90.0, VAN_PUT}
    };
    SimInput market_conditions = {
        parse_time("2026-08-10"), 10,  // start_ts, dt,
        100.0, 60.00, 0.05, 0.03,           // S0, vol, rate, div_yield
        800.00,                              // drift
        "AAPL"
    };
}
