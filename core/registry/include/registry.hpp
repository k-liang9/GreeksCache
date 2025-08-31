#ifndef REGISTRY
#define REGISTRY

#include <vector>
#include <array>
#include <chrono>
#include <string>
#include <unordered_map>
#include <boost/lockfree/spsc_queue.hpp>
#include "types.hpp"

using namespace std;
using namespace boost::lockfree;

enum MissingLevel {
    EXISTS = 0,
    SYMBOL = 1,
    EXPIRY = 2,
    CONTRACT = 3
};

struct ContractKey {
    double strike;
    PayoffType payoff_type;
    
    bool operator==(const ContractKey& other) const {
        return strike == other.strike && 
               payoff_type == other.payoff_type;
    }
    
    bool operator<(const ContractKey& other) const {
        if (strike != other.strike) return strike < other.strike;
        return payoff_type < other.payoff_type;
    }
};

struct ContractKeyHash {
    size_t operator()(const ContractKey& key) const {
        return std::hash<double>()(key.strike) ^
               (std::hash<int>()(static_cast<int>(key.payoff_type)) << 1);
    }
};

class UniverseRegistry {
private:
    unordered_map<string, size_t> symbol_to_id_;
    vector<string> id_to_symbol_;
    vector<unordered_map<t_ns, size_t>> expiry_to_id_;
    vector<vector<t_ns>> id_to_expiry_;

    vector<vector<unordered_map<ContractKey, size_t, ContractKeyHash>>> contract_to_id_;
    vector<vector<vector<ContractKey>>> id_to_contract_;
    size_t epoch_;

    void update_epoch() { epoch_++; }
    MissingLevel missing_level_of(Contract& contract);
    void track_new_symbol(string& symbol);
    void add_new_expiry(size_t symbol_id, t_ns expiry_ns);
    void add_new_contract(size_t symbol_id, size_t expiry_id, float strike, PayoffType payoff_type);

public:
    UniverseRegistry() : epoch_(0) {}
    UniverseRegistry(vector<Contract>& contracts);

    void flush_changes(spsc_queue<Contract>& open, spsc_queue<Contract>& close);
    void add_contracts(vector<Contract>& contracts);
    static EngineType engine_of(PayoffType type);

    const unordered_map<string, size_t>& get_symbol_to_id() { return symbol_to_id_; }
    const vector<string>& get_id_to_symbol() { return id_to_symbol_; }
    const vector<unordered_map<t_ns, size_t>>& get_expiry_to_id() { return expiry_to_id_; }
    const vector<vector<t_ns>>& get_id_to_expiry() { return id_to_expiry_; }
    const auto& get_contract_to_id() { return contract_to_id_; }
    const auto& get_id_to_contract() { return id_to_contract_; }
    const size_t epoch() { return epoch_; }
};

#endif