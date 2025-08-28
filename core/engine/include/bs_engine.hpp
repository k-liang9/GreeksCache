#ifndef BS_ENGINE
#define BS_ENGINE

#include <vector>
#include <array>
#include "engine.hpp"
#include "hot_state_types.hpp"

using namespace std;

class BsEngine : public IEngine {
private:
    MarketSnapshot* snapshot_;
    SliceContext* context_;
    BatchInputs* inputs_;
    KernelScratch* scratch_;
    Greeks* greeks_;

public:
    BsEngine() {}

    void evaluate(
        MarketSnapshot& snapshot, SliceContext& context, 
        BatchInputs& inputs, KernelScratch& scratch, Greeks& greeks
    ) override;
private:

    void evaluate_vanilla(const pair<size_t, size_t>& vanilla_range);
};

#endif