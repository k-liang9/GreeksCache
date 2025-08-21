#ifndef MARKET_DATA
#define MARKET_DATA

#include <string>

using namespace std;

struct MarketData {
    string symbol;
    double spot;
    double volatility; //FIXME: currently assuming const volatility. Need to implement IV surface later
};

#endif