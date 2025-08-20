#include "testing_vars.hpp"

using namespace std;

namespace test {
    vector<double> spot = {100.0, 102.0, 98.0, 101.0, 99.0, 103.0};
    vector<double> K = {100.0, 105.0, 110.0, 95.0, 120.0, 90.0};
    vector<string_view> T = {"2026-08-20T16:00:00Z", "2026-09-20T16:00:00Z", "2026-10-20T16:00:00Z", "2026-11-20T16:00:00Z", "2026-12-20T16:00:00Z", "2027-01-20T16:00:00Z"};
    double risk_free_rate = 0.05;
    vector<double> vol = {0.2, 0.4, 0.15, 0.33, 0.7, 0.05};
    vector<double> dividend = {0.0, 0.02, 0.01, 0.015, 0.025, 0.005};
}
