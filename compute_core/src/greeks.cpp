#include "greeks.hpp"
#include "testing_vars.hpp"
#include <cmath>
#include <chrono>
#include <vector>
#include <numbers>
#include <iomanip>
#include <string_view>
#include <iostream>

using namespace std;

void compute_call_greeks(vector<double>& greeks, float S, string_view T, float K, float sigma, float q, float r) {
    tm exp_time;
    parse_time(T, exp_time);
    float tau = time_to_expiry(exp_time);
    double d1 = 1/(sigma*sqrt(tau)) * (log(S/K) + (r - q + pow(sigma, 2)/2) * tau);
    double d2 = d1 - sigma * sqrt(tau);
    double q_disc = exp(- q * tau);
    double r_disc = exp(- r * tau);
    double Nd1 = N(d1);
    double nd1 = n(d1);
    double Nd2 = N(d2);
    double nd2 = n(d2);
    double asset = S * q_disc * Nd1;
    double strike = K * r_disc * Nd2;

    double call_price = asset - strike;
    double Delta = q_disc * Nd1;
    double Gamma = q_disc / (S * sigma * sqrt(tau)) * nd1;
    double Vega = S * q_disc * sqrt(tau) * nd1 / 100;
    double Rho = K * tau * r_disc * Nd2 / 100;
    double Theta = (- S * q_disc * sigma / (2*sqrt(tau)) * nd1 - r * strike + q * asset) / 365.0;

    greeks = {call_price, Delta, Gamma, Vega, Rho, Theta};
}

void compute_put_greeks(vector<double>& greeks, float S, string_view T, float K, float sigma, float q, float r) {
    tm exp_time;
    parse_time(T, exp_time);
    float tau = time_to_expiry(exp_time);
    double d1 = 1/(sigma*sqrt(tau)) * (log(S/K) + (r - q + pow(sigma, 2)/2) * tau);
    double d2 = d1 - sigma * sqrt(tau);
    double q_disc = exp(- q * tau);
    double r_disc = exp(- r * tau);
    double Nd1 = N(-d1);
    double nd1 = n(d1);
    double Nd2 = N(-d2);
    double nd2 = n(d2);
    double asset = S * q_disc * Nd1;
    double strike = K * r_disc * Nd2;

    double put_price = strike - asset;
    double Delta = -q_disc * Nd1;
    double Gamma = q_disc / (S * sigma * sqrt(tau)) * nd1;
    double Vega = S * q_disc * sqrt(tau) * nd1 / 100;
    double Rho = -K * tau * r_disc * Nd2 / 100;
    double Theta = (-S * q_disc * nd1 * sigma / (2 * sqrt(tau)) + r * strike - q * asset) / 365.0;

    greeks = {put_price, Delta, Gamma, Vega, Rho, Theta};
}

const double N(const double x) {
    return 0.5 * std::erfc(-x / std::sqrt(2.0));
}

const double n(const double x) {
    return 1/sqrt(2 * M_PI) * exp(- pow(x, 2) / 2);
}

float time_to_expiry(const tm& T) {
    using namespace std::chrono;
    auto now_tp = system_clock::now();
    std::time_t now_c = system_clock::to_time_t(now_tp);
    std::tm now = *std::gmtime(&now_c);    

    int delta_day = T.tm_yday - now.tm_yday;
    int delta_year;
    if (delta_day < 0) {
        delta_day += 365;
        delta_year = T.tm_year - 1 - now.tm_year;
    } else {
        delta_year = T.tm_year - now.tm_year;
    }

    return delta_year + delta_day/365.0;
}

void parse_time(string_view T, tm& exp_time) {
    exp_time = {};

    auto to_int = [&](size_t pos, size_t len) -> int {
        int v = 0;
        for (size_t i = 0; i < len; ++i) {
            char c = T[pos + i];
            v = v * 10 + (c - '0');
        }
        return v;
    };

    int year  = to_int(0, 4);
    int month = to_int(5, 2);   // 1-12
    int day   = to_int(8, 2);   // 1-31
    int hour  = to_int(11, 2);  // 0-23
    int min   = to_int(14, 2);  // 0-59
    int sec   = to_int(17, 2);  // 0-60

    exp_time.tm_year = year - 1900;
    exp_time.tm_mon  = month - 1; // 0-11
    exp_time.tm_mday = day;
    exp_time.tm_hour = hour;
    exp_time.tm_min  = min;
    exp_time.tm_sec  = sec;
    exp_time.tm_isdst = 0; // UTC

    // Compute day-of-year (tm_yday): days since Jan 1, [0, 365]
    auto is_leap = [&](int y) {
        return (y % 4 == 0) && ((y % 100 != 0) || (y % 400 == 0));
    };
    static const int cum_days_norm[12] = { 0, 31, 59, 90,120,151,181,212,243,273,304,334 };
    int doy = cum_days_norm[month - 1] + (day - 1);
    if (is_leap(year) && month > 2) {
        ++doy;
    }
    exp_time.tm_yday = doy;
}