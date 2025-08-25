#ifndef GBM_SIM
#define GBM_SIM

#include <string>
#include "utils.hpp"
#include "market_data.hpp"
#include "types.hpp"

using namespace std;

void generate_bm_path(vector<pair<t_ns, double>>& path, t_ns start, t_ms dt, t_s sim_duration);


class GbmSimulator {
private:
    t_ns start_ts_;
    t_ms dt_;
    t_s sim_duration_;
    double S0_;
    double vol_ms_;
    double rate_;
    double div_yield_;
    double drift_ms_;
    string symbol_;
    
    void generate_gbm_path(vector<pair<t_ns, double>>& path);
public:
    GbmSimulator() {};

    void generate_sim_data(vector<MarketData>& sim_data);
    void input_sim_data(
        t_ns start, t_ms dt, t_s sim_duration,
        double S0, double vol_annual, double rate, double div_yield,
        double drift_annual,
        string symbol
    );
};

#endif