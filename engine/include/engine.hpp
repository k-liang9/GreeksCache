#ifndef ENGINE
#define ENGINE

#include "hot_state_types.hpp"

class IEngine {
public:
    virtual void evaluate(MarketSnapshot& snapshot, SliceContext& context, BatchInputs& inputs, KernelScratch& scratch, Greeks& greeks);
};

#endif