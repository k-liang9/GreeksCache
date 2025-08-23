#ifndef REGISTRY
#define REGISTRY

#include <vector>
#include <array>
#include <chrono>
#include <string>
#include <unordered_map>
#include "options.hpp"

using namespace std;
using contract_t = tuple<size_t, double, PayoffType>;

class UniverseRegistry {
private:
    unordered_map<string, size_t> symbol_to_id;
    vector<string> id_to_symbol;
    vector<unordered_map<string, size_t>> expiry_to_id;
    vector<vector<string>> id_to_expiry;

    vector<unordered_map<contract_t, size_t>> contract_to_id;
    vector<vector<contract_t>> id_to_contract;

public:
    UniverseRegistry() {}
    UniverseRegistry(vector<Contract>& contracts);

    void add_contracts(vector<Contract>& contracts);
    static EngineType engine_of(PayoffType type);
    void contract_to_string(
        string& contract, 
        const size_t expiry_id, 
        const double strike, 
        const PayoffType payoff_type);

    const unordered_map<string, size_t>& symbol_to_id() const { return symbol_to_id; }
    const vector<string>& id_to_symbol() const { return id_to_symbol; }
    const vector<unordered_map<string, size_t>>& expiry_to_id() const { return expiry_to_id; }
    const vector<vector<string>>& id_to_expiry() const { return id_to_expiry; }
    const vector<unordered_map<contract_t, size_t>>& contract_to_id() const { return contract_to_id; }
    const vector<vector<contract_t>>& id_to_contract() const { return id_to_contract; }
};

#endif