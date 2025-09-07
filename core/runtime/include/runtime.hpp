#ifndef RUNTIME
#define RUNTIME

#include <vector>
#include "types.hpp"

using namespace std;

void run_core();
bool core_ready();
void enqueue_contracts(vector<Contract>& contracts);


#endif