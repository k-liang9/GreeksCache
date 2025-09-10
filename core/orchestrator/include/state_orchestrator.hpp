#ifndef STATE_ORCHESTRATOR
#define STATE_ORCHESTRATOR

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <boost/lockfree/spsc_queue.hpp>
#include "symbol_state.hpp"
#include "registry.hpp"
#include "redis_publisher.hpp"
#include "types.hpp"

constexpr float BATCH_HEADROOM = 0.2;

using namespace std;
using namespace boost::lockfree;

class SymbolState;

class StateOrchestrator {
private:
    UniverseRegistry registry_;
    RedisPublisher redis_publisher_;

    unordered_map<size_t, unique_ptr<SymbolState>> symbol_table_;
    spsc_queue<Contract> opened_contracts_;

    void build_and_publish_jobs(unique_ptr<SymbolState>& symbol_state, MarketData& market_data);
    vector<pair<size_t, size_t>> build_expiry_batch(vector<PayoffType>& payoff_types, vector<double>& strikes);
    
    void retire_expiry_slices();
    void ensure_symbol_state(size_t symbol_id);
    void check_batch_need(
        unique_ptr<SymbolState>& symbol_state,
        const ContractMeta& contract_meta,
        bool& need_new_batch,
        string& reason
    );
    void remove_old_batch(
        unique_ptr<SymbolState>& symbol_state,
        const ContractMeta& contract_meta
    );
    void create_or_recreate_batch(
        unique_ptr<SymbolState>& symbol_state,
        const ContractMeta& contract_meta
    );
    void add_to_existing_batch(
        unique_ptr<SymbolState>& symbol_state,
        const ContractMeta& contract_meta
    );
public:
    StateOrchestrator();

    void flush_changes();
    bool enqueue_contract(Contract& contracts);
    void initialize_state(vector<Contract>& contracts);
    void process_tick(MarketData& market_data);

    const unordered_map<size_t, unique_ptr<SymbolState>>& symbol_table() const { return symbol_table_; }
    RedisPublisher& redis_publisher() { return redis_publisher_; }
};

#endif