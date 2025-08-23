#include <iostream>
#include <vector>
#include "bs_engine.hpp"
#include "testing_vars.hpp"

using namespace std;

int main() {
    vector<array<double, 6>> greeks;
    string symbol = "AAPL";
    ContractsBook contracts_book{};
    for (Contract& contract : test::contracts) {
        contracts_book.add_contract(contract);
    }

    ContractsBatch& AAPL_contracts = contracts_book.contracts_book()[symbol];
    BsEngine::compute_greeks(AAPL_contracts, test::market_data);

    const array<const char*, NUM_OPTION_TYPES> type_names = {"CALL", "PUT"};
    for (int type = 0; type < NUM_OPTION_TYPES; ++type) {
        const auto& prices = AAPL_contracts.prices_[type];
        const auto& deltas = AAPL_contracts.deltas_[type];
        const auto& gammas = AAPL_contracts.gammas_[type];
        const auto& vegas  = AAPL_contracts.vegas_[type];
        const auto& rhos   = AAPL_contracts.rhos_[type];
        const auto& thetas = AAPL_contracts.thetas_[type];
        cout << type_names[type] << " contracts:" << endl;
        for (size_t i = 0; i < prices.size(); ++i) {
            cout << "  Contract " << i + 1 << ":" << endl;
            cout << "    Price=" << prices[i] << endl;
            cout << "    Delta=" << deltas[i] << endl;
            cout << "    Gamma=" << gammas[i] << endl;
            cout << "    Vega=" << vegas[i] << endl;
            cout << "    Rho=" << rhos[i] << endl;
            cout << "    Theta=" << thetas[i] << endl;
        }
        cout << endl;
    }
    return 0;
}