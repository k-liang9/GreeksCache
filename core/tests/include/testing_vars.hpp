#ifndef TESTING_VARS
#define TESTING_VARS

#include <string>
#include <vector>
#include <string_view>
#include "types.hpp"

using namespace std;

namespace test {
    struct SimInput {
        t_ns start_ts;
        t_ms dt;
        double S0, vol, rate, div_yield;
        double drift;
        string symbol;
    };

    extern string test_expiry;
    extern vector<Contract> user_changes;
    extern vector<Contract> contracts;
    extern SimInput apple_market_conditions;
    extern SimInput google_market_conditions;
}

#endif