#ifndef EXPIRY_STATE
#define EXPIRY_STATE

#include <numbers>
#include <vector>
#include <array>
#include <memory>
#include "options.hpp"
#include "engine.hpp"
#include "types.hpp"

class IEngine;
class CacheProvider;

using namespace std;

template<typename T>
class DoubleBuffer {
private:
    array<vector<T>, 2> buffers;
    size_t write_index = 0;

public:
    vector<T>& write() { return buffers[write_index]; }
    const vector<T>& read() const { return buffers[1 - write_index]; }
    vector<T>* write_ptr() { return &buffers[write_index]; }
    const vector<T>* read_ptr() const { return &buffers[1 - write_index]; }
    
    void swap() { write_index = 1 - write_index; }
    
    void resize(size_t size) {
        buffers[0].resize(size);
        buffers[1].resize(size);
    }
};

class IExpiryBatch {
protected:
    //id
    const size_t expiry_id_;
    const t_ns expiry_ts_ns_;
    const EngineType engine_type_;

    //shared slice
    double tau_years, sqrt_tau;
    double disc_r, disc_q;
    t_ns as_of_ns;
    size_t seqno;

    //static SoA
    vector<double> strikes_;
    vector<PayoffType> payoff_types_;
    vector<pair<size_t, size_t>> payoff_ranges_;
    
    unique_ptr<IEngine> engine_;

    virtual void prepare_tick(double spot, double vol, double rate, double div_yield);

public:
    DoubleBuffer<double> theo, delta, gamma, vega, rho, theta;

    IExpiryBatch(size_t expiry_id, t_ns expiry_ns, EngineType engine_type, 
                 vector<double> strikes, vector<PayoffType> payoff_types,
                 vector<pair<size_t, size_t>> ranges);

    void process_tick(double spot, double vol, double rate, double div_yield);
    void swap_buffers();
};

class BSBatch : public IExpiryBatch {
public:
    BSBatch(size_t expiry_id, t_ns expiry_ns, EngineType engine_type, 
            vector<double> strikes, vector<PayoffType> payoff_types,
            vector<pair<size_t, size_t>> ranges);

private:
    vector<double> d1, d2;
    vector<int> vanilla_type_mask;
    pair<size_t, size_t> vanilla_range;

protected:
    void prepare_tick(double spot, double vol, double rate, double div_yield) override;
};

#endif