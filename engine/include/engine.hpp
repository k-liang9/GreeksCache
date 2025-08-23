#ifndef ENGINE
#define ENGINE

#include "expiry_state.hpp"

using namespace std;

class IEngine {
public:
    virtual void evaluate();
};

#endif