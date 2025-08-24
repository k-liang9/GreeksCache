#include "testing_vars.hpp"
#include "utils.hpp"
#include "options.hpp"

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
    MarketData market_data = {
        "AAPL",
        103.0,
        0.05,
        0.025,
        0.2,
        now()
    };
}
