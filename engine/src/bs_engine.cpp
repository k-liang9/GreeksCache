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
#include "bs_engine.hpp"

using namespace std;

namespace BsEngine {
    void compute_greeks(ContractsBatch& contracts, MarketData& market_data) {
        const auto& strikes = contracts.strikes();
        const auto& expiries = contracts.expiries();
        int num_types = strikes.size();
        for (size_t i = 0; i < num_types; i++) {
            switch (i) {
                case VAN_CALL:
                    eur_call_greeks(contracts, market_data);
                    break;
                case VAN_PUT:
                    eur_put_greeks(contracts, market_data);
                    break;
                default:
                    cout << "invalid option type";
            }
        }
    }

    void eur_call_greeks(ContractsBatch& contracts, MarketData& market_data) {
        assert(market_data.symbol == contracts.symbol());

        const auto& strikes = contracts.strikes()[VAN_CALL];
        const auto& expiries = contracts.expiries()[VAN_CALL];
        auto& prices = contracts.prices_[VAN_CALL];
        auto& deltas = contracts.deltas_[VAN_CALL];
        auto& gammas = contracts.gammas_[VAN_CALL];
        auto& vegas = contracts.vegas_[VAN_CALL];
        auto& rhos = contracts.rhos_[VAN_CALL];
        auto& thetas = contracts.thetas_[VAN_CALL];
        const auto& vols = market_data.vols[VAN_CALL];
        size_t num_contracts = strikes.size();

        assert(expiries.size() == num_contracts);
        assert(vols.size() == num_contracts);

        prices.clear();
        deltas.clear();
        gammas.clear();
        vegas.clear();
        rhos.clear();
        thetas.clear();

        vector<double> taus;
        tm now;
        now(now);
        time_to_expiry(expiries, now, taus);
        
        for (size_t i = 0; i < num_contracts; i++) {
            double S = market_data.spot;
            double K = strikes[i];
            double tau = taus[i];
            double sigma = vols[i];
            double q = market_data.div_yield;
            double r = market_data.rate;
            double d1 = 1/(sigma * sqrt(tau)) * (log(S/K) + (r - q + pow(sigma, 2)/2) * tau);
            double d2 = d1 - sigma * sqrt(tau);
            double q_disc = exp(- q * tau);
            double r_disc = exp(- r * tau);
            double nd1 = n(d1);
            double nd2 = n(d2);
            double Nd1 = N(d1);
            double Nd2 = N(d2);
            double asset = S * q_disc * Nd1;
            double strike = K * r_disc * Nd2;


            double call_price = asset - strike;
            double Delta = q_disc * Nd1;
            double Gamma = q_disc * nd1 / (S * sigma * sqrt(tau));
            double Vega = S * q_disc * sqrt(tau) * nd1 / 100;
            double Rho = K * tau * r_disc * Nd2 / 100;
            double Theta = (- S * q_disc * sigma / (2*sqrt(tau)) * nd1 - r * strike + q * asset) / 365.0;

            prices.push_back(call_price);            
            deltas.push_back(Delta);       
            gammas.push_back(Gamma);        
            vegas.push_back(Vega);        
            rhos.push_back(Rho);        
            thetas.push_back(Theta);            
        }
    }

    void eur_put_greeks(ContractsBatch& contracts, MarketData& market_data) {
        assert(market_data.symbol == contracts.symbol());

        const auto& strikes = contracts.strikes()[VAN_PUT];
        const auto& expiries = contracts.expiries()[VAN_PUT];
        auto& prices = contracts.prices_[VAN_PUT];
        auto& deltas = contracts.deltas_[VAN_PUT];
        auto& gammas = contracts.gammas_[VAN_PUT];
        auto& vegas = contracts.vegas_[VAN_PUT];
        auto& rhos = contracts.rhos_[VAN_PUT];
        auto& thetas = contracts.thetas_[VAN_PUT];
        const auto& vols = market_data.vols[VAN_PUT];
        size_t num_contracts = strikes.size();

        assert(expiries.size() == num_contracts);
        assert(vols.size() == num_contracts);

        prices.clear();
        deltas.clear();
        gammas.clear();
        vegas.clear();
        rhos.clear();
        thetas.clear();

        vector<double> taus;
        tm now;
        now(now);
        time_to_expiry(expiries, now, taus);
        
        for (size_t i = 0; i < num_contracts; i++) {
            double S = market_data.spot;
            double K = strikes[i];
            double tau = taus[i];
            double sigma = vols[i];
            double q = market_data.div_yield;
            double r = market_data.rate;

            double d1 = 1/(sigma * sqrt(tau)) * (log(S/K) + (r - q + pow(sigma, 2)/2) * tau);
            double d2 = d1 - sigma * sqrt(tau);
            double q_disc = exp(- q * tau);
            double r_disc = exp(- r * tau);
            double nd1 = n(d1);
            double nd2 = n(d2);
            double Nd1 = N(-d1);
            double Nd2 = N(-d2);
            double asset = S * q_disc * Nd1;
            double strike = K * r_disc * Nd2;
            
            double put_price = strike - asset;
            double Delta = -q_disc * Nd1;
            double Gamma = q_disc * nd1 / (S * sigma * sqrt(tau));
            double Vega = S * q_disc * sqrt(tau) * nd1 / 100;
            double Rho = -K * tau * r_disc * Nd2 / 100;
            double Theta = (-S * q_disc * nd1 * sigma / (2 * sqrt(tau)) + r * strike - q * asset) / 365.0;

            prices.push_back(put_price);            
            deltas.push_back(Delta);       
            gammas.push_back(Gamma);        
            vegas.push_back(Vega);        
            rhos.push_back(Rho);        
            thetas.push_back(Theta);      
        }
    }
}