#ifndef SYMBOL_STATE
#define SYMBOL_STATE

#include <numbers>
#include <vector>
#include <memory>
#include "options.hpp"
#include "expiry_state.hpp"

using namespace std;

class SymbolState {
public:
    const size_t symbol_id;
    vector<unique_ptr<IExpiryBatch>> batches; //1 expiry + 1 engine
    double spot;
    size_t spot_as_of_ns;
    size_t seqno;
    size_t calibration_version;

    SymbolState(size_t symbol_id) : symbol_id(symbol_id) {}
};

#endif