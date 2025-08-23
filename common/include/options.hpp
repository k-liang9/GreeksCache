#ifndef OPTIONS
#define OPTIONS

enum PayoffType {
    VAN_CALL,
    VAN_PUT,
    NUM_OPTION_TYPES,
    PAYOFFTYPE_ERROR
};

enum EngineType {
    BS_ANALYTIC,
    NUM_ENGINE_TYPES,
    ENGINETYPE_ERROR
};

enum GreeksIndex {
    DELTA,
    GAMMA,
    VEGA,
    RHO,
    THETA,
    NUM_GREEKS
};

#endif