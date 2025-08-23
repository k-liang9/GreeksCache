#ifndef CONTRACTS
#define CONTRACTS

#include <vector>
#include <array>
#include <chrono>
#include <string>
#include <unordered_map>
#include "options.hpp"

using namespace std;

struct Contract {
    string symbol;
    string expiry;
    float strike;
    PayoffType payoff_type;
};

class UniverseRegistry {
private:
    unordered_map<string, size_t> symbol_to_id;
    vector<string> id_to_symbol;
    vector<unordered_map<string, size_t>> expiry_to_id;
    vector<vector<string>> id_to_expiry;

    vector<unordered_map<string, size_t>> contract_to_id;
    vector<vector<string>> id_to_contract;

    void contract_to_string(
        string& contract, 
        const size_t expiry_id, 
        const double strike, 
        const PayoffType payoff_type);
    EngineType engine_of(PayoffType type);
public:
    UniverseRegistry() {}
    UniverseRegistry(vector<Contract>& contracts);

    void add_contracts(vector<Contract>& contracts);
};

#endif