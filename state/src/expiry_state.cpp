#include <memory>
#include <iostream>
#include "expiry_state.hpp"
#include "bs_engine.hpp"
#include "utils.hpp"
#include "hot_state_types.hpp"

using namespace std;
class BsEngine;

IExpiryBatch::IExpiryBatch(size_t expiry_id, t_ns expiry_ns, EngineType engine_type, vector<double> strikes, vector<PayoffType> payoff_types, vector<pair<size_t, size_t>> ranges) :
expiry_id_(expiry_id),
expiry_ts_ns_(expiry_ns),
engine_type_(engine_type),
strikes_(std::move(strikes)),
payoff_types_(std::move(payoff_types)),
contract_ranges_(std::move(ranges)) {
    
    switch (engine_type_) {
    case BS_ANALYTIC:
        engine_ = make_unique<BsEngine>();
        break;
    default:
        engine_ = nullptr;
        break;
    }
    num_contracts_ = strikes_.size();
    
    theo_.resize(num_contracts_);
    delta_.resize(num_contracts_);
    gamma_.resize(num_contracts_);
    vega_.resize(num_contracts_);
    rho_.resize(num_contracts_);
    theta_.resize(num_contracts_);
}

void IExpiryBatch::swap_buffers() {
    theo_.swap();
    delta_.swap();
    gamma_.swap();
    vega_.swap();
    rho_.swap();
    theta_.swap();
}

void IExpiryBatch::process_tick(MarketSnapshot& snapshot) {
    tau = ns_to_yrs(expiry_ts_ns_ - snapshot.as_of_ns);
    sqrt_tau = sqrt(tau);
    disc_r = exp(-snapshot.rate * tau);
    disc_q = exp(-snapshot.div_yield * tau);
    seqno_ = snapshot.seqno;

    SliceContext context = {tau, sqrt_tau, disc_r, disc_q};
    BatchInputs inputs = compile_batch_inputs();
    KernelScratch scratch = prepare_tick(snapshot);
    
    Greeks greeks = {
        theo_.write(), delta_.write(), gamma_.write(),
        vega_.write(), rho_.write(), theta_.write()};

    engine_->evaluate(snapshot, context, inputs, scratch, greeks);
    swap_buffers();
}

BSBatch::BSBatch(size_t expiry_id, t_ns expiry_ns, EngineType engine_type, vector<double> strikes, vector<PayoffType> payoff_types, vector<pair<size_t, size_t>> ranges) :
IExpiryBatch(expiry_id, expiry_ns, engine_type, std::move(strikes), std::move(payoff_types), std::move(ranges)) {
    fill(d1_.begin(), d1_.end(), 0.0);
    fill(d2_.begin(), d2_.end(), 0.0);
    
    vanilla_type_mask_.resize(num_contracts_);
    
    for (auto& range : contract_ranges_) {
        if (range.first < payoff_types_.size() && (payoff_types_[range.first] == VAN_CALL || payoff_types_[range.first] == VAN_PUT)) {
            for (size_t i = range.first; i < range.second && i < payoff_types_.size(); i++) {
                if (payoff_types_[i] == VAN_CALL) {
                    vanilla_type_mask_[i] = 1;
                } else if (payoff_types_[i] == VAN_PUT) {
                    vanilla_type_mask_[i] = -1;
                } else {
                    vanilla_type_mask_[i] = 0;
                }
            }
        }
    }
}

KernelScratch BSBatch::prepare_tick(MarketSnapshot& data) {
    // Resize vectors to match the number of strikes
    size_t num_strikes = strikes_.size();
    d1_.resize(num_strikes);
    d2_.resize(num_strikes);

    double S = data.spot;
    double sigma = data.vol;
    double r = data.rate;
    double q = data.div_yield;

    for (const pair<size_t, size_t>& range : contract_ranges_) {
        for (size_t i = range.first; i < range.second && i < strikes_.size(); i++) {
            double K = strikes_[i];
            if (sigma > 0 && sqrt_tau > 0) {
                d1_[i] = (log(S/K) + (r - q + pow(sigma, 2)/2) * tau) / (sigma * sqrt_tau);
                d2_[i] = d1_[i] - sigma * sqrt_tau;
            } else {
                d1_[i] = 0.0;
                d2_[i] = 0.0;
            }
        }
    }

    return {d1_, d2_};
}

BatchInputs BSBatch::compile_batch_inputs() {
    return {strikes_, payoff_types_, contract_ranges_, vanilla_type_mask_};
}