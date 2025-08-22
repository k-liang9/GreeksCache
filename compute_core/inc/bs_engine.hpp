#ifndef BS_ENGINE
#define BS_ENGINE

#include <vector>
#include <string_view>
#include <array>
#include "contracts.hpp"
#include "market_data.hpp"

using namespace std;

namespace BsEngine {
    void compute_greeks(ContractsBatch& contracts, MarketData& market_data);
    void eur_call_greeks(ContractsBatch& contracts, MarketData& market_data);
    void eur_put_greeks(ContractsBatch& contracts, MarketData& market_data);
}

#endif