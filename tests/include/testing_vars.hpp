#ifndef TESTING_VARS
#define TESTING_VARS

#include <string>
#include <vector>
#include <string_view>
#include "options.hpp"
#include "market_data.hpp"

using namespace std;

namespace test {
    struct SimInput {
        t_ns start_ts;
        t_ms dt;
        t_s sim_duration;
        double S0, vol, rate, div_yield;
        double drift;
        string symbol;
    };

    extern vector<Contract> contracts;
    extern SimInput market_conditions;
}

#endif