#include <iostream>
#include <vector>
#include "greeks.hpp"
#include "testing_vars.hpp"

using namespace std;

int main() {
    vector<double> greeks;
    compute_put_greeks(greeks, test::spot, test::T, test::K, test::vol, test::dividend, test::risk_free_rate);
    for (float f : greeks) {
        cout << f << '\n';
    }
    return 0;
}