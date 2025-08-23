#ifndef MARKET_DATA
#define MARKET_DATA

#include <string>
#include <vector>
#include <array>
#include "options.hpp"
#include "types.hpp"

using namespace std;

struct MarketData {
    string symbol;
    double spot;
    double rate;
    double div_yield;
    double vol;
    t_ns ts_ns;
};

#endif