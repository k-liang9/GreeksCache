#ifndef SYMBOL_STATE
#define SYMBOL_STATE

#include <numbers>
#include <vector>
#include <memory>
#include "options.hpp"
#include "market_data.hpp"
#include "expiry_state.hpp"
#include "types.hpp"

using namespace std;

class SymbolState {
private:
    const size_t symbol_id_;
    vector<unique_ptr<IExpiryBatch>> batches_; //1 expiry + 1 engine
    double spot_;
    t_ns spot_as_of_ns_;
    size_t seqno_;

    //TODO: currently flat values, change to continuous surfaces/curves from market data
    size_t calibration_version_; //calibrated curves (IV, etc)
    double vol_;
    double rate_;
    double div_yield_;

    void update_seqno() { seqno_++; }
public:
    SymbolState(size_t symbol_id) : symbol_id_(symbol_id) {}
    void process_tick(MarketData& market_data);
    void add_expiry_batch(size_t expiry_id, t_ns expiry_ns, EngineType engine_type, 
                         vector<double> strikes, vector<PayoffType> payoff_types,
                         vector<pair<size_t, size_t>> ranges);
};

#endif