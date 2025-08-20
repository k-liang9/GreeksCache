#include <cmath>
#include <chrono>
#include <vector>
#include <array>
#include <string_view>
#include <string>
#include <iostream>
#include <cassert>
#include <experimental/simd>
#include "utils.hpp"
#include "testing_vars.hpp"
#include "eur_greeks.hpp"

using namespace std;

void EurContractBatch::compute_greeks(
    vector<array<double, 6>>& greeks,
    const vector<double>& S_vec,
    const vector<double>& K_vec,
    const vector<string_view>& T_vec,
    const vector<double>& sigma_vec,
    const vector<double>& q_vec,
    const double r
) {
    assert(S_vec.size() == K_vec.size() &&
            K_vec.size() == T_vec.size() &&
            T_vec.size() == sigma_vec.size() &&
            sigma_vec.size() == q_vec.size());
    
    tm now;
    get_cur_time(now);
    Constants constants;
    time_to_expiry(T_vec, now, constants.tau_vec);
    
    for (size_t i = 0; i < S_vec.size(); i++) {
        double S = S_vec[i];
        double K = K_vec[i];
        double tau = constants.tau_vec[i];
        double sigma = sigma_vec[i];
        double q = q_vec[i];

        double d1 = 1/(sigma * sqrt(tau)) * (log(S/K) + (r - q + pow(sigma, 2)/2) * tau);
        double d2 = d1 - sigma * sqrt(tau);
        double q_disc = exp(- q * tau);
        double r_disc = exp(- r * tau);
        double nd1 = n(d1);
        double nd2 = n(d2);
        double Gamma = q_disc * nd1 / (S * sigma * sqrt(tau));
        double Vega = S * q_disc * sqrt(tau) * nd1 / 100;

        constants.d1_vec.push_back(d1);
        constants.d2_vec.push_back(d2);
        constants.q_disc_vec.push_back(q_disc);
        constants.r_disc_vec.push_back(r_disc);
        constants.nd1_vec.push_back(nd1);
        constants.nd2_vec.push_back(nd2);
        constants.Gamma.push_back(Gamma);
        constants.Vega.push_back(Vega);
    }
    
    compute_greeks_impl(greeks, S_vec, K_vec, sigma_vec, q_vec, r, constants);
}

void EurCallContractBatch::compute_greeks_impl(
    vector<array<double, 6>>& greeks,
    const vector<double>& S_vec,
    const vector<double>& K_vec,
    const vector<double>& sigma_vec,
    const vector<double>& q_vec,
    const double r,
    const Constants& constants
) {
    size_t num_contracts = S_vec.size();
    for (size_t i = 0; i < num_contracts; i++) {
        double S = S_vec[i];
        double K = K_vec[i];
        double sigma = sigma_vec[i];
        double q = q_vec[i];
        double d1 = constants.d1_vec[i];
        double d2 = constants.d2_vec[i];
        double nd1 = constants.nd1_vec[i];
        double nd2 = constants.nd2_vec[i];
        double r_disc = constants.r_disc_vec[i];
        double q_disc = constants.q_disc_vec[i];
        double tau = constants.tau_vec[i];
        double Gamma = constants.Gamma[i];
        double Vega = constants.Vega[i];

        double Nd1 = N(d1);
        double Nd2 = N(d2);
        double asset = S * q_disc * Nd1;
        double strike = K * r_disc * Nd2;
        double call_price = asset - strike;
        double Delta = q_disc * Nd1;
        double Rho = K * tau * r_disc * Nd2 / 100;
        double Theta = (- S * q_disc * sigma / (2*sqrt(tau)) * nd1 - r * strike + q * asset) / 365.0;

        greeks.push_back({call_price, Delta, Gamma, Vega, Rho, Theta});
    }
}

void EurPutContractBatch::compute_greeks_impl(
    vector<array<double, 6>>& greeks,
    const vector<double>& S_vec,
    const vector<double>& K_vec,
    const vector<double>& sigma_vec,
    const vector<double>& q_vec,
    const double r,
    const Constants& constants
) {
    size_t num_contracts = S_vec.size();
    for (size_t i = 0; i < num_contracts; i++) {
        double S = S_vec[i];
        double K = K_vec[i];
        double sigma = sigma_vec[i];
        double q = q_vec[i];
        double d1 = constants.d1_vec[i];
        double d2 = constants.d2_vec[i];
        double nd1 = constants.nd1_vec[i];
        double nd2 = constants.nd2_vec[i];
        double r_disc = constants.r_disc_vec[i];
        double q_disc = constants.q_disc_vec[i];
        double tau = constants.tau_vec[i];
        double Gamma = constants.Gamma[i];
        double Vega = constants.Vega[i];

        double Nd1 = N(-d1);
        double Nd2 = N(-d2);
        double asset = S * q_disc * Nd1;
        double strike = K * r_disc * Nd2;
        double put_price = strike - asset;
        double Delta = -q_disc * Nd1;
        double Rho = -K * tau * r_disc * Nd2 / 100;
        double Theta = (-S * q_disc * nd1 * sigma / (2 * sqrt(tau)) + r * strike - q * asset) / 365.0;

        greeks.push_back({put_price, Delta, Gamma, Vega, Rho, Theta});
    }
}