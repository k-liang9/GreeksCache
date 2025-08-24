#ifndef EXPIRY_STATE
#define EXPIRY_STATE

#include <numbers>
#include <vector>
#include <array>
#include <memory>
#include "options.hpp"
#include "engine.hpp"
#include "types.hpp"
#include "market_data.hpp"
#include "hot_state_types.hpp"

class IEngine;
class CacheProvider;

using namespace std;

class IExpiryBatch {
protected:
    //id
    const size_t expiry_id_;
    const t_ns expiry_ts_ns_;
    const EngineType engine_type_;

    //shared slice
    double tau, sqrt_tau;
    double disc_r, disc_q;
    size_t seqno_;
    size_t num_contracts_;

    //static SoA
    vector<double> strikes_;
    vector<PayoffType> payoff_types_;
    vector<pair<size_t, size_t>> contract_ranges_;

    DoubleBuffer<double> theo_, delta_, gamma_, vega_, rho_, theta_;
    
    unique_ptr<IEngine> engine_;

    virtual KernelScratch prepare_tick(MarketSnapshot& snapshot) = 0;
    virtual BatchInputs compile_batch_inputs() = 0;

public:
    IExpiryBatch(size_t expiry_id, t_ns expiry_ns, EngineType engine_type, 
                 vector<double> strikes, vector<PayoffType> payoff_types,
                 vector<pair<size_t, size_t>> ranges);
    
    virtual ~IExpiryBatch() = default;

    void process_tick(MarketSnapshot& snapshot);
    void swap_buffers();
    const vector<double>& strikes() { return strikes_; }
    const vector<PayoffType>& payoff_types() { return payoff_types_; }
    const size_t expiry_id() { return expiry_id_; }
    const t_ns expiry_ts_ns() { return expiry_ts_ns_; }
    const EngineType engine_type() { return engine_type_; }
    const unique_ptr<IEngine>& engine() { return engine_; }
};

class BSBatch : public IExpiryBatch {
public:
    BSBatch(size_t expiry_id, t_ns expiry_ns, EngineType engine_type, 
            vector<double> strikes, vector<PayoffType> payoff_types,
            vector<pair<size_t, size_t>> ranges);

private:
    vector<double> d1_, d2_;
    vector<int> vanilla_type_mask_;

protected:
    KernelScratch prepare_tick(MarketSnapshot& snapshot) override;
    BatchInputs compile_batch_inputs() override;
};

#endif