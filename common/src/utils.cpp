#include "utils.hpp"
#include <string_view>
#include <vector>
#include <chrono>
#include <cmath>

using namespace std;

void time_to_expiry(const vector<tm>& Ts, const tm& now, vector<double>& tau) {
    for (auto& T : Ts) {
        int delta_day = T.tm_yday - now.tm_yday;
        int delta_year;
        if (delta_day < 0) {
            delta_day += 365;
            delta_year = T.tm_year - 1 - now.tm_year;
        } else {
            delta_year = T.tm_year - now.tm_year;
        }

        tau.push_back(delta_year + delta_day/365.0);
    }
}

t_ns now() {
    return static_cast<t_ns>(
        chrono::duration_cast<chrono::nanoseconds>(
            chrono::system_clock::now().time_since_epoch()
        ).count()
    );
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

const double N(const double x) {
    return 0.5 * std::erfc(-x / std::sqrt(2.0));
}

const double n(const double x) {
    return 1/sqrt(2 * M_PI) * exp(- pow(x, 2) / 2);
}