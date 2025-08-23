#ifndef REGISTRY
#define REGISTRY

#include <vector>
#include <array>
#include <chrono>
#include <string>
#include <unordered_map>
#include "options.hpp"
#include "types.hpp"

using namespace std;

struct ContractKey {
    size_t expiry_id;
    double strike;
    PayoffType payoff_type;
    
    bool operator==(const ContractKey& other) const {
        return expiry_id == other.expiry_id && 
               strike == other.strike && 
               payoff_type == other.payoff_type;
    }
    
    bool operator<(const ContractKey& other) const {
        if (expiry_id != other.expiry_id) return expiry_id < other.expiry_id;
        if (strike != other.strike) return strike < other.strike;
        return payoff_type < other.payoff_type;
    }
};

struct ContractKeyHash {
    size_t operator()(const ContractKey& key) const {
        return std::hash<size_t>()(key.expiry_id) ^ 
               (std::hash<double>()(key.strike) << 1) ^
               (std::hash<int>()(static_cast<int>(key.payoff_type)) << 2);
    }
};

class UniverseRegistry {
private:
    unordered_map<string, size_t> symbol_to_id;
    vector<string> id_to_symbol;
    vector<unordered_map<t_ns, size_t>> expiry_to_id;
    vector<vector<t_ns>> id_to_expiry;

    vector<unordered_map<ContractKey, size_t, ContractKeyHash>> contract_to_id;
    vector<vector<ContractKey>> id_to_contract;

public:
    UniverseRegistry() {}
    UniverseRegistry(vector<Contract>& contracts);

    void add_contracts(vector<Contract>& contracts);
    static EngineType engine_of(PayoffType type);

    const unordered_map<string, size_t>& get_symbol_to_id() const { return symbol_to_id; }
    const vector<string>& get_id_to_symbol() const { return id_to_symbol; }
    const vector<unordered_map<t_ns, size_t>>& get_expiry_to_id() const { return expiry_to_id; }
    const vector<vector<t_ns>>& get_id_to_expiry() const { return id_to_expiry; }
    const vector<unordered_map<ContractKey, size_t, ContractKeyHash>>& get_contract_to_id() const { return contract_to_id; }
    const vector<vector<ContractKey>>& get_id_to_contract() const { return id_to_contract; }
};

#endif