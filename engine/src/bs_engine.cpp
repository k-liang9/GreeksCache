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

void BsEngine::evaluate(MarketSnapshot& snapshot, SliceContext& context, BatchInputs& inputs, KernelScratch& scratch, Greeks& greeks) {
    snapshot_ = &snapshot;
    context_ = &context;
    inputs_ = &inputs;
    scratch_ = &scratch;
    greeks_ = &greeks;

    for (auto& range : inputs.ranges) {
        if (inputs.payoff_types[range.first] == VAN_CALL || inputs.payoff_types[range.first] == VAN_PUT) {
            evaluate_vanilla(range);
        }
    }

    snapshot_ = nullptr;
    context_ = nullptr;
    inputs_ = nullptr;
    scratch_ = nullptr;
    greeks_ = nullptr;
}

void BsEngine::evaluate_vanilla(const pair<size_t, size_t>& vanilla_range) {
    double S = snapshot_->spot;
    double sigma = snapshot_->vol;
    double r = snapshot_->rate;
    double q = snapshot_->div_yield;
    double tau = context_->tau;
    double sqrt_tau = context_->sqrt_tau;
    double disc_r = context_->disc_r;
    double disc_q = context_->disc_q;
    const vector<int>& mask = inputs_->vanilla_type_mask;

    for (size_t i = vanilla_range.first; i < vanilla_range.second; i++) {
        int mask_val = mask[i - vanilla_range.first];
        double d1 = scratch_->d1[i];
        double d2 = scratch_->d2[i];
        double nd1 = n(d1);
        double nd2 = n(d2);
        double Nd1 = N(mask_val * d1);
        double Nd2 = N(mask_val * d2);
        double K = inputs_->strikes[i];
        double asset_term = S * disc_q * Nd1;
        double strike_term = K * disc_r * Nd2;
        
        greeks_->theo[i] = mask_val * (asset_term - strike_term);
        greeks_->delta[i] = mask_val * (disc_q * Nd1);
        greeks_->gamma[i] = disc_q * nd1 / (S * sigma * sqrt_tau);
        greeks_->vega[i] = S * disc_q * sqrt_tau * nd1 / 100;
        greeks_->rho[i] = mask_val * K * tau * disc_r * Nd2 / 100;
        greeks_->theta[i] = (
            - S * disc_q * sigma / (2*sqrt_tau) * nd1 
            - r * strike_term * mask_val
            + q * asset_term * mask_val
        ) / 365.0;
    } 
}