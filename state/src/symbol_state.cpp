#include <memory>
#include "symbol_state.hpp"
#include "market_data.hpp"

using namespace std;

void SymbolState::process_tick(MarketData& market_data) {
    spot_ = market_data.spot;
    spot_as_of_ns_ = market_data.ts_ns;
    vol_ = market_data.vol;
    div_yield_ = market_data.div_yield;
    rate_ = market_data.rate;
    update_seqno();

    for (auto& batch : batches_) {
        batch->process_tick(spot_, vol_, rate_, div_yield_);
    }
}

void SymbolState::add_expiry_batch(
    size_t expiry_id,
    t_ns expiry_ns,
    EngineType engine_type,
    vector<double> strikes,
    vector<PayoffType> payoff_types,
    vector<pair<size_t, size_t>> ranges) {
    batches_.emplace_back(make_unique<BSBatch>(expiry_id, expiry_ns, engine_type, move(strikes), move(payoff_types), move(ranges)));
}