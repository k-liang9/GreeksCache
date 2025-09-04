#include <random>
#include <cmath>
#include <thread>
#include <iostream>
#include <boost/lockfree/spsc_queue.hpp>
#include "mailbox.hpp"
#include "gbm_sim.hpp"
#include "utils.hpp"
#include "types.hpp"

using namespace std;

random_device rd{};
mt19937 gen{42};
constexpr double ms_per_yr = 365.25 * 24 * 60 * 60 * 1000.0;

GbmSimulator::GbmSimulator(t_ns start, t_ms dt,
        double S0, double vol_annual, double rate, double div_yield,
        double drift_annual,
        string symbol) :
    start_ts_(start),
    dt_(dt),
    S0_(S0),
    rate_(rate),
    div_yield_(div_yield),
    vol_(vol_annual),
    vol_ms_(vol_annual / sqrt(ms_per_yr)),
    drift_ms_(drift_annual / (ms_per_yr)),
    symbol_(symbol),
    W_t_(0.0),
    t_ms_(0),
    dist_(0.0, sqrt((double)dt_)) {}

void GbmSimulator::run(boost::lockfree::spsc_queue<MarketData>& market_data_queue) {
    W_t_ += dist_(gen);
    t_ms_ += dt_;
    double spot = S0_ * exp((drift_ms_ - pow(vol_ms_, 2)/2) * t_ms_ + vol_ms_ * W_t_);
    MarketData tick_data = {symbol_, spot, vol_, rate_, div_yield_, start_ts_ + ms_to_ns(t_ms_)};

    if (spot <= 0.0 || std::isnan(spot)) {
        cerr << "[GbmSimulator WARNING] generated spot=" << spot
             << " S0=" << S0_ << " t_ms=" << t_ms_ << " W_t=" << W_t_ << '\n';
    }
    if (market_data_queue.read_available()) {
        cout << "system cannot keep up with constant data stream of " << dt_ << "ms\n";
    }
    market_data_queue.push(tick_data);

    this_thread::sleep_for(chrono::milliseconds(dt_));
}

void GbmSimulator::run(MarketMailbox& mailbox) {
    W_t_ += dist_(gen);
    t_ms_ += dt_;
    double spot = S0_ * exp((drift_ms_ - pow(vol_ms_, 2)/2) * t_ms_ + vol_ms_ * W_t_);
    MarketData tick_data = {symbol_, spot, vol_, rate_, div_yield_, start_ts_ + ms_to_ns(t_ms_)};

    if (spot <= 0.0 || std::isnan(spot)) {
        cerr << "[GbmSimulator WARNING] generated spot=" << spot
             << " S0=" << S0_ << " t_ms=" << t_ms_ << " W_t=" << W_t_ << '\n';
    }
    mailbox.publish(tick_data);

    this_thread::sleep_for(chrono::milliseconds(dt_));
}