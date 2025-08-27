#ifndef TYPES
#define TYPES

#include <cstddef>
#include <vector>
using namespace std;

using t_ns = size_t;
using t_s = size_t;
using t_ms = size_t;

enum PayoffType {
    VAN_CALL,
    VAN_PUT,
    NUM_OPTION_TYPES,
    VANILLA,
    PAYOFFTYPE_ERROR
};

enum EngineType {
    BS_ANALYTIC,
    NUM_ENGINE_TYPES,
    ENGINETYPE_ERROR
};

struct Contract {
    string symbol;
    string expiry;
    double strike;
    PayoffType payoff_type;
};

struct MarketData {
    string symbol;
    double spot;
    double rate;
    double div_yield;
    double vol;
    t_ns ts_ns;
};

struct PublishJob {
    string symbol;
    t_ns as_of_ns;
    double spot, vol, rate, div_yield;
    size_t calibration_version, seqno;
    t_ns expiry;
    EngineType engine_type;
    const vector<double>* strikes;
    const vector<PayoffType>* payoff_types;
    const vector<pair<size_t, size_t>>* ranges;
    vector<double> theo, delta, gamma, vega, rho, theta;
};

#endif
