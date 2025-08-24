#ifndef ENGINE
#define ENGINE

#include "hot_state_types.hpp"

class IEngine {
public:
    virtual ~IEngine() = default;
    
    virtual void evaluate(
        MarketSnapshot& snapshot, SliceContext& context, 
        BatchInputs& inputs, KernelScratch& scratch, Greeks& greeks
    ) = 0;
};

#endif