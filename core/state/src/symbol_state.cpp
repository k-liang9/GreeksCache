#include <memory>
#include <iostream>
#include "symbol_state.hpp"
#include "types.hpp"

using namespace std;

SymbolState::SymbolState(size_t symbol_id)
    : symbol_id_(symbol_id) {}

void SymbolState::process_tick(MarketData& data) {
    spot_ = data.spot;
    as_of_ns_ = data.ts_ns;
    vol_ = data.vol;
    div_yield_ = data.div_yield;
    rate_ = data.rate;
    update_seqno();

    MarketSnapshot snapshot = {spot_, vol_, rate_, div_yield_, as_of_ns_, seqno_};

    for (auto& [expiry_id, expiry_vec] : batches_) {
        for (auto& batch : expiry_vec) {
            batch->process_tick(snapshot);
        }
    }
}

void SymbolState::add_expiry_batch(
    size_t expiry_id,
    t_ns expiry_ns,
    EngineType engine_type,
    vector<double> strikes,
    vector<PayoffType> payoff_types,
    vector<pair<size_t, size_t>> ranges) {
    batches_[expiry_id].emplace_back(make_unique<BSBatch>(expiry_id, expiry_ns, engine_type, std::move(strikes), std::move(payoff_types), std::move(ranges)));
}

void SymbolState::retire_expiry_slice(size_t expiry_id) {
    batches_.erase(expiry_id);
}