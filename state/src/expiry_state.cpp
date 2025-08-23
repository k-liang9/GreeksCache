#include <memory>
#include "expiry_state.hpp"
#include "bs_engine.hpp"

using namespace std;
class BsEngine;

IExpiryBatch::IExpiryBatch(size_t expiry_id, t_ns expiry_ns, EngineType engine_type, vector<double> strikes, vector<PayoffType> payoff_types, vector<pair<size_t, size_t>> ranges) :
expiry_id_(expiry_id),
expiry_ts_ns_(expiry_ns),
engine_type_(engine_type),
strikes_(move(strikes)),
payoff_types_(move(payoff_types)),
payoff_ranges_(move(ranges)) {
    
    switch (engine_type_) {
    case BS_ANALYTIC:
        engine_ = make_unique<BsEngine>();
        break;
    default:
        engine_ = nullptr;
        break;
    }
    size_t num_contracts = strikes_.size();
    
    theo.resize(num_contracts);
    delta.resize(num_contracts);
    gamma.resize(num_contracts);
    vega.resize(num_contracts);
    rho.resize(num_contracts);
    theta.resize(num_contracts);
}

void IExpiryBatch::swap_buffers() {
    theo.swap();
    delta.swap();
    gamma.swap();
    vega.swap();
    rho.swap();
    theta.swap();
}

void IExpiryBatch::process_tick(double spot, double vol, double rate, double div_yield) {
    prepare_tick(spot, vol, rate, div_yield);
    engine_->evaluate();
}

BSBatch::BSBatch(size_t expiry_id, t_ns expiry_ns, EngineType engine_type, vector<double> strikes, vector<PayoffType> payoff_types, vector<pair<size_t, size_t>> ranges) :
IExpiryBatch(expiry_id, expiry_ns, engine_type, move(strikes), move(payoff_types), move(ranges)) {

}

void BSBatch::prepare_tick(double spot, double vol, double rate, double div_yield) {

}