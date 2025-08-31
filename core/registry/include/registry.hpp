#ifndef REGISTRY
#define REGISTRY

#include <vector>
#include <array>
#include <chrono>
#include <string>
#include <unordered_map>
#include <queue>
#include <functional>
#include <boost/lockfree/spsc_queue.hpp>
#include "types.hpp"

#define GRACE_PERIOD_NS s_to_ns(30)
#define STRIKE_SCALE 10000

using namespace std;
using namespace boost::lockfree;

enum MissingLevel {
    EXISTS = 0,
    SYMBOL = 1,
    EXPIRY = 2,
    CONTRACT = 3
};

struct ContractKey {
    int strike_scaled;
    PayoffType payoff_type;
    
    bool operator==(const ContractKey& other) const {
        return strike_scaled == other.strike_scaled && 
               payoff_type == other.payoff_type;
    }
    
    bool operator<(const ContractKey& other) const {
        if (strike_scaled != other.strike_scaled) return strike_scaled < other.strike_scaled;
        return payoff_type < other.payoff_type;
    }
};

struct ContractKeyHash {
    size_t operator()(const ContractKey& key) const {
        return std::hash<double>()(key.strike_scaled) ^
               (std::hash<int>()(static_cast<int>(key.payoff_type)) << 1);
    }
};

enum ExpiryStatus {
    ACTIVE,
    RETIRED
};

struct ExpiryMeta {
    size_t expiry_id;
    t_ns expiry_ns;
    ExpiryStatus status = ACTIVE;
    t_ns retired_at = 0;
};

class UniverseRegistry {
private:
    unordered_map<string, size_t> symbol_to_id_;
    vector<string> id_to_symbol_;
    vector<unordered_map<t_ns, size_t>> expiry_to_id_;
    vector<vector<t_ns>> id_to_expiry_;

    vector<vector<unordered_map<ContractKey, size_t, ContractKeyHash>>> contract_to_id_;
    vector<vector<vector<ContractKey>>> id_to_contract_;

    vector<vector<ExpiryMeta>> expiry_metas_;
    vector<unordered_map<t_ns, ExpiryMeta*>> ns_to_meta_;
    vector<priority_queue<t_ns, vector<t_ns>, std::greater<t_ns>>> expiry_queues_;

    size_t epoch_;

    void update_epoch() { epoch_++; }
    MissingLevel missing_level_of(Contract& contract);
    void track_new_symbol(string& symbol);
    void add_new_expiry(size_t symbol_id, t_ns expiry_ns);
    void add_new_contract(size_t symbol_id, size_t expiry_id, float strike, PayoffType payoff_type);
    void flush_user_changes(spsc_queue<Contract>& open, spsc_queue<Contract>& close);
    void retire_expired_slices();

public:
    UniverseRegistry() : epoch_(0) {}
    UniverseRegistry(vector<Contract>& contracts);

    void update_registry(spsc_queue<Contract>& open, spsc_queue<Contract>& close);
    void add_contracts(vector<Contract>& contracts);
    static EngineType engine_of(PayoffType type);

    const unordered_map<string, size_t>& get_symbol_to_id() { return symbol_to_id_; }
    const vector<string>& get_id_to_symbol() { return id_to_symbol_; }
    const vector<unordered_map<t_ns, size_t>>& get_expiry_to_id() { return expiry_to_id_; }
    const vector<vector<t_ns>>& get_id_to_expiry() { return id_to_expiry_; }
    const auto& get_contract_to_id() { return contract_to_id_; }
    const auto& get_id_to_contract() { return id_to_contract_; }
    const vector<vector<ExpiryMeta>>& get_expiry_metas() { return expiry_metas_; }
    const vector<unordered_map<t_ns, ExpiryMeta*>>& get_ns_to_meta() { return ns_to_meta_; }
    const vector<priority_queue<t_ns, vector<t_ns>, std::greater<t_ns>>>& get_expiry_queues() { return expiry_queues_; }

    const size_t epoch() { return epoch_; }
};

#endif