#include <memory>
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
strikes_(move(strikes)),
payoff_types_(move(payoff_types)),
contract_ranges_(move(ranges)) {
    
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
    prepare_tick(snapshot);
    as_of_ns_ = now();
    tau = ns_to_yrs(expiry_ts_ns_ - as_of_ns_);
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
IExpiryBatch(expiry_id, expiry_ns, engine_type, move(strikes), move(payoff_types), move(ranges)) {
    d1.resize(num_contracts_);
    d2.resize(num_contracts_);
    vanilla_type_mask.resize(num_contracts_);
    for (auto& range : contract_ranges_) {
        if (payoff_types_[range.first] == VAN_CALL) {
            fill(vanilla_type_mask.begin() + range.first, vanilla_type_mask.begin() + range.second, 1);
        } else if (payoff_types_[range.first] == VAN_PUT) {
            fill(vanilla_type_mask.begin() + range.first, vanilla_type_mask.begin() + range.second, -1);
        }
    }
}

KernelScratch BSBatch::prepare_tick(MarketSnapshot& data) {
    d1.clear();
    d2.clear();

    double S = data.spot;
    double sigma = data.vol;
    double r = data.rate;
    double q = data.div_yield;

    for (pair<size_t, size_t>& range : contract_ranges_) {
        for (size_t i = range.first; i < range.second; i++) {
            double K = strikes_[i];
            d1[i] = 1 / (sigma * sqrt_tau) * (log(S/K) + (r - q + pow(sigma, 2)/2) * tau);
            d2[i] = d1[i] - sigma * sqrt_tau;  
        }
    }

    return {d1, d2};
}

BatchInputs BSBatch::compile_batch_inputs() {
    return {strikes_, payoff_types_, contract_ranges_, vanilla_type_mask};
}