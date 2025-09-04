#ifndef GBM_SIM
#define GBM_SIM

#include <string>
#include <random>
#include <boost/lockfree/spsc_queue.hpp>
#include "mailbox.hpp"
#include "utils.hpp"
#include "types.hpp"

using namespace std;

class GbmSimulator {
private:
    const t_ns start_ts_;
    const t_ms dt_;
    const double S0_;
    const double vol_ms_;
    const double vol_;
    const double rate_;
    const double div_yield_;
    const double drift_ms_;
    const string symbol_;
    normal_distribution<double> dist_;

    double W_t_;
    t_ms t_ms_;
    
public:
    GbmSimulator(t_ns start, t_ms dt,
        double S0, double vol_annual, double rate, double div_yield,
        double drift_annual,
        string symbol);
    void run(boost::lockfree::spsc_queue<MarketData>& market_data_queue); //FIXME: probably only for benchmarking
    void run(MarketMailbox& mailbox);
};

#endif