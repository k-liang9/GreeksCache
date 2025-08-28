#ifndef STATE_ORCHESTRATOR
#define STATE_ORCHESTRATOR

#define BATCH_HEADROOM 0.2;

#include <vector>
#include <unordered_map>
#include <functional>
#include <memory>
#include <queue>
#include "symbol_state.hpp"
#include "registry.hpp"
#include "redis_publisher.hpp"
#include "types.hpp"

using namespace std;

class SymbolState;

class StateOrchestrator {
private:
    UniverseRegistry registry_;
    RedisPublisher redis_publisher_;

    unordered_map<size_t, unique_ptr<SymbolState>> symbol_table_;
    std::queue<Contract> changes;

    void build_and_publish_jobs(unique_ptr<SymbolState>& symbol_state, MarketData& market_data);
    vector<pair<size_t, size_t>> build_expiry_batch(vector<PayoffType>& payoff_types, vector<double>& strikes);
public:
    StateOrchestrator();

    void add_contracts(vector<Contract>& contracts);
    void initialize_state(vector<Contract>& contracts);
    void process_tick(MarketData& market_data);
    const unordered_map<size_t, unique_ptr<SymbolState>>& symbol_table() const { return symbol_table_; }
    RedisPublisher& redis_publisher() { return redis_publisher_; }
};

#endif