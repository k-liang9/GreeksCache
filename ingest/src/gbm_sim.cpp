#include <random>
#include <cmath>
#include "gbm_sim.hpp"
#include "utils.hpp"

using namespace std;

random_device rd{};
mt19937 gen{42};

void generate_bm_path(
    vector<pair<t_ns, double>>& path,
    t_ns start, t_ms dt, t_s sim_duration
) {
    normal_distribution dist{0.0, sqrt((double)dt)};
    t_ns end = start + s_to_ns(sim_duration);
    t_ns dt_ns = ms_to_ns(dt);
    double B = 0;
    path.push_back({start, 0.0});
    for (t_ns t = start + dt_ns; t <= end; t += dt_ns) {
        B += dist(gen);
        path.push_back({t, B});
    }
}

void GbmSimulator::generate_gbm_path(vector<pair<t_ns, double>>& path) {
    generate_bm_path(path, start_ts_, dt_, sim_duration_);
    for (size_t i = 0; i < path.size(); i++) {
        double W_t = path[i].second;
        double t_ms = i * dt_;
        double S_t = S0_ * exp((drift_ms_ - pow(vol_ms_, 2)/2) * t_ms + vol_ms_ * W_t);
        path[i].second = S_t;
    }
}

void GbmSimulator::generate_sim_data(vector<MarketData>& sim_data) {
    vector<pair<t_ns, double>> path;
    generate_gbm_path(path);
    for (auto& tick : path) {
        MarketData tick_data = {symbol_, tick.second, rate_, div_yield_, vol_ms_, tick.first};
        sim_data.emplace_back(std::move(tick_data));
    }
}

void GbmSimulator::input_sim_data(
    t_ns start, t_ms dt, t_s sim_duration,
    double S0, double vol_annual, double rate, double div_yield,
    double drift_annual,
    string symbol
) {
    start_ts_ = start;
    dt_ = dt;
    sim_duration_ = sim_duration;
    S0_ = S0;
    rate_ = rate;
    div_yield_ = div_yield;
    symbol_ = symbol;
    
    // Convert annual parameters to millisecond scale
    const double ms_in_year = 365.25 * 24 * 60 * 60 * 1000.0;
    // Scale up volatility for more visible movement in testing
    vol_ms_ = vol_annual / sqrt(ms_in_year) * 1000.0;   // Amplify by 1000x for testing
    drift_ms_ = drift_annual / ms_in_year * 1000.0;     // Amplify by 1000x for testing
}