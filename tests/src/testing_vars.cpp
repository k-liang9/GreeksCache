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
        parse_time("2025-08-20"), 100, 5,  // start_ts, dt, sim_duration
        100.0, 0.20, 0.05, 0.03,           // S0, vol (20% annual), rate, div_yield
        0.05,                              // drift (5% annual)
        "AAPL"
    };
}
