#ifndef SYMBOL_STATE
#define SYMBOL_STATE

#include <numbers>
#include <vector>
#include <memory>
#include <unordered_map>
#include "expiry_state.hpp"
#include "types.hpp"
#include "hot_state_types.hpp"
#include "symbol_state.hpp"

using namespace std;

class SymbolState {
private:
    const size_t symbol_id_;
    unordered_map<size_t, vector<unique_ptr<IExpiryBatch>>> batches_; //keyed by expiry_id
    double spot_;
    t_ns as_of_ns_;
    size_t seqno_;

    //TODO: currently flat values, change to continuous surfaces/curves from market data
    size_t calibration_version_; //calibrated curves (IV, etc)
    double vol_;
    double rate_;
    double div_yield_;
    
    void update_seqno() { seqno_++; }
public:
    SymbolState(size_t symbol_id);
    void process_tick(MarketData& market_data);
    void add_expiry_batch(
        size_t expiry_id, t_ns expiry_ns, EngineType engine_type, 
        vector<double> strikes, vector<PayoffType> payoff_types,
        vector<pair<size_t, size_t>> ranges);
    void retire_expiry_slice(size_t expiry_id);

    const auto& batches() { return batches_; }
    const size_t symbol_id() { return symbol_id_; }
    const t_ns as_of_ns() { return as_of_ns_; }
    const size_t seqno() { return seqno_; }
    const size_t calibration_version() { return calibration_version_; }
};

#endif