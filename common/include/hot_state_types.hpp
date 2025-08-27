#ifndef HOT_STATE_TYPE
#define HOT_STATE_TYPE

#include <vector>
#include "types.hpp"

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


struct MarketSnapshot {
    double spot;
    double vol, rate, div_yield;
    t_ns as_of_ns;
    size_t seqno;
};

struct SliceContext {
    double tau, sqrt_tau;
    double disc_r, disc_q;
};

struct BatchInputs {
    const vector<double>& strikes;
    const vector<PayoffType>& payoff_types;
    const vector<pair<size_t, size_t>>& ranges;
    const vector<int>& vanilla_type_mask; //BsEngine only
};

struct KernelScratch {
    const vector<double>& d1, d2; //BsEngine only
};

struct Greeks {
    vector<double> &theo, &delta, &gamma, &vega, &rho, &theta;
};

#endif