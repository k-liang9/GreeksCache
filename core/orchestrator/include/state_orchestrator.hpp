#ifndef STATE_ORCHESTRATOR
#define STATE_ORCHESTRATOR

#define BATCH_HEADROOM 0.2;

#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <boost/lockfree/spsc_queue.hpp>
#include "symbol_state.hpp"
#include "registry.hpp"
#include "redis_publisher.hpp"
#include "types.hpp"

using namespace std;
using namespace boost::lockfree;

class SymbolState;

class StateOrchestrator {
private:
    UniverseRegistry registry_;
    RedisPublisher redis_publisher_;

    unordered_map<size_t, unique_ptr<SymbolState>> symbol_table_;
    spsc_queue<Contract> opened_contracts_;
    spsc_queue<Contract> closed_contracts_;

    void build_and_publish_jobs(unique_ptr<SymbolState>& symbol_state, MarketData& market_data);
    vector<pair<size_t, size_t>> build_expiry_batch(vector<PayoffType>& payoff_types, vector<double>& strikes);
public:
    StateOrchestrator();

    void flush_changes();
    void sink_contract_changes(bool add, vector<Contract>& contracts);
    void initialize_state(vector<Contract>& contracts);
    void process_tick(MarketData& market_data);

    const unordered_map<size_t, unique_ptr<SymbolState>>& symbol_table() const { return symbol_table_; }
    RedisPublisher& redis_publisher() { return redis_publisher_; }
};

#endif