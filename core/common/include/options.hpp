#ifndef OPTIONS
#define OPTIONS

#include <string>

using namespace std;

enum PayoffType {
    VAN_CALL,
    VAN_PUT,
    NUM_OPTION_TYPES,
    VANILLA,
    PAYOFFTYPE_ERROR
};

enum EngineType {
    BS_ANALYTIC,
    NUM_ENGINE_TYPES,
    ENGINETYPE_ERROR
};

struct Contract {
    string symbol;
    string expiry;
    double strike;
    PayoffType payoff_type;
};

#endif