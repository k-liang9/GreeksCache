#include <iostream>
#include <vector>
#include "eur_greeks.hpp"
#include "testing_vars.hpp"

using namespace std;

int main() {
    vector<array<double, 6>> greeks;
    EurPutContractBatch batch = EurPutContractBatch();
    batch.compute_greeks(greeks, test::spot, test::K, test::T, test::vol, test::dividend, test::risk_free_rate);
    for (size_t i = 0; i < greeks.size(); ++i) {
        const auto& greek_array = greeks[i];
        cout << "Contract " << i + 1 << ":" << endl;
        cout << "  Price: " << greek_array[0] << endl;
        cout << "  Delta: " << greek_array[1] << endl;
        cout << "  Gamma: " << greek_array[2] << endl;
        cout << "  Vega:  " << greek_array[3] << endl;
        cout << "  Rho:   " << greek_array[4] << endl;
        cout << "  Theta: " << greek_array[5] << endl;
        cout << endl;
    }
    return 0;
}