#include "testing_vars.hpp"
#include "utils.hpp"

using namespace std;

namespace test {
    vector<Contract> contracts = {
        {"AAPL", "2026-08-20T16:00:00Z", 100.0, EUR_CALL},
        {"AAPL", "2026-9-20T16:00:00Z", 105.0, EUR_CALL},
        {"AAPL", "2026-10-20T16:00:00Z", 110.0, EUR_CALL},
        {"AAPL", "2026-11-20T16:00:00Z", 95.0, EUR_CALL},
        {"AAPL", "2026-12-20T16:00:00Z", 120.0, EUR_CALL},
        {"AAPL", "2027-01-20T16:00:00Z", 90.0, EUR_CALL}
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
