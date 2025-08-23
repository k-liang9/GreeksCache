#ifndef TESTING_VARS
#define TESTING_VARS

#include <string>
#include <vector>
#include <string_view>
#include "options.hpp"
#include "market_data.hpp"

using namespace std;

namespace test {
    extern vector<Contract> contracts;
    extern MarketData market_data;
}

#endif