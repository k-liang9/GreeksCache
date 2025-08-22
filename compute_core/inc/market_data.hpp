#ifndef MARKET_DATA
#define MARKET_DATA

#include <string>
#include <vector>
#include "contracts.hpp"

using namespace std;

struct MarketData {
    string symbol;
    double spot;
    double rate;
    double div_yield;
    array<vector<double>, NUM_OPTION_TYPES> vols; //FIXME: currently giving a dummy vector for volatility. Need to implement IV surface later
};

#endif