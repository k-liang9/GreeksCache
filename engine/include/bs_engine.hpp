#ifndef BS_ENGINE
#define BS_ENGINE

#include <vector>
#include <array>
#include "engine.hpp"
#include "hot_state_types.hpp"

using namespace std;

// namespace BsEngine {
//     void compute_greeks(ContractsBatch& contracts, MarketData& market_data);
//     void eur_call_greeks(ContractsBatch& contracts, MarketData& market_data);
//     void eur_put_greeks(ContractsBatch& contracts, MarketData& market_data);
// }

class BsEngine : public IEngine {
public:
    void evaluate(MarketSnapshot& snapshot, SliceContext& context, BatchInputs& inputs, KernelScratch& scratch, Greeks& greeks) override;
};

#endif