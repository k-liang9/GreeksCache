#ifndef EXPIRY_STATE
#define EXPIRY_STATE

#include <numbers>
#include <vector>
#include "options.hpp"

class IEngine;

using namespace std;

class IExpiryBatch {
protected:
    //id
    const size_t expiry_id;
    const size_t expiry_ts_ns;
    const EngineType engine_type;

    //shared slice
    double tau_years, sqrt_tau;
    double disc_r, disc_q;
    size_t as_of_ns, seqno;

    //static SoA
    vector<double> strikes;
    vector<PayoffType> payoff_type;

    IEngine* engine;

    void prepare_tick();
    void publish(CacheProvider& cache_provider); //push results to redis;

public:
    vector<double> theo_0, delta_0, gamma_0, vega_0, rho_0, theta_0;
    vector<double> theo_1, delta_1, gamma_1, vega_1, rho_1, theta_1;
    vector<double>* theo_read, delta_read, gamma_read, vega_read, rho_read, theta_read;
    vector<double>* theo_write, delta_write, gamma_write, vega_write, rho_write, theta_write; //TODO: assign these at constructor, also use std::swap
    byte epoch_bit;

    IExpiryBatch(size_t expiry_id, EngineType engine_type, vector<double> strikes, vector<PayoffType> payoff_types);
};

class BSBatch : public IExpiryBatch {
public:
    BSBatch(size_t expiry_id, EngineType engine_type, vector<double> strikes, vector<PayoffType> payoff_types);

private:
    vector<double> d1, d2;
    vector<int> vanilla_type_mask;
    pair<size_t, size_t> vanilla_range;
};

#endif