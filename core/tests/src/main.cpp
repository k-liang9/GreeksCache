#include <iostream>
#include <vector>
#include <iomanip>
#include <atomic>
#include <thread>
#include <chrono>
#include <unordered_map>
#include <iterator>
#include <boost/lockfree/spsc_queue.hpp>
#include <sw/redis++/redis++.h>
#include "bs_engine.hpp"
#include "testing_vars.hpp"
#include "types.hpp"
#include "registry.hpp"
#include "state_orchestrator.hpp"
#include "utils.hpp"
#include "gbm_sim.hpp"
#include "mailbox.hpp"

using namespace std;
using namespace sw::redis;

bool g_ready = false;

const bool core_ready() {
    return g_ready;
}

int main() {
    StateOrchestrator orchestrator = StateOrchestrator();
    orchestrator.initialize_state(test::contracts);
    GbmSimulator apple_sim = GbmSimulator(
        test::apple_market_conditions.start_ts,
        test::apple_market_conditions.dt,
        test::apple_market_conditions.S0,
        test::apple_market_conditions.vol,
        test::apple_market_conditions.rate,
        test::apple_market_conditions.div_yield,
        test::apple_market_conditions.drift,
        test::apple_market_conditions.symbol
    );
    GbmSimulator google_sim = GbmSimulator(
        test::google_market_conditions.start_ts,
        test::google_market_conditions.dt,
        test::google_market_conditions.S0,
        test::google_market_conditions.vol,
        test::google_market_conditions.rate,
        test::google_market_conditions.div_yield,
        test::google_market_conditions.drift,
        test::google_market_conditions.symbol
    );
    //print_contract_listing(orchestrator);
    // boost::lockfree::spsc_queue<MarketData> apple_market_data_stream{128};
    // boost::lockfree::spsc_queue<MarketData> google_market_data_stream{128};
    MarketMailbox apple_mb{};
    MarketMailbox google_mb{};
    size_t apple_seq = 0;
    size_t google_seq = 0;

    atomic<bool> stop{false};

    thread market_sim([&]{
        while (!stop.load()) {
            apple_sim.run(apple_mb);
            google_sim.run(google_mb);
        }
    });

    thread compute_core([&]{
        MarketData data;
        auto last_flush = chrono::steady_clock::now();
        while (!stop.load()) {
            if (apple_mb.read_if_updated(data, apple_seq) || google_mb.read_if_updated(data, google_seq)) {
                auto now = chrono::steady_clock::now();
                orchestrator.flush_changes();
                last_flush = now;

                orchestrator.process_tick(data);
                // print_greeks_tick(data, orchestrator);
            } else {
                this_thread::sleep_for(chrono::milliseconds(5));
            }
            
            // Flush changes every 5 seconds
            auto now = chrono::steady_clock::now();
            if (chrono::duration_cast<chrono::seconds>(now - last_flush).count() >= 5) {
                orchestrator.flush_changes();
                last_flush = now;
            }
        }
    });

    thread publisher([&]{
        while (!stop.load()) {
            orchestrator.redis_publisher().run();
        }
    });

    thread reader([&]{
        auto redis = Redis("tcp://localhost:6379");
        while (!stop.load()) {
            try {
                unordered_map<string, string> result;
                redis.hgetall("greeks:GOOGL:" + test::test_expiry + ":251.0000:VAN_PUT", inserter(result, result.begin()));
                if (!result.empty()) {
                    cout << "=== REDIS HGETALL RESULT ===" << endl;
                    cout << "key: greeks:GOOGL:" + test::test_expiry + ":251.0000:VAN_PUT\n";
                    for (const auto& pair : result) {
                        cout << pair.first << ": " << pair.second << endl;
                    }
                    cout << endl;
                }
            } catch (const exception& e) {
                // Redis connection might not be ready yet, continue silently
            }
            this_thread::sleep_for(chrono::seconds(2));
        }
    });

    thread user_changes([&]{
        this_thread::sleep_for(chrono::seconds(3));
        orchestrator.sink_contract_changes(test::user_changes);
    });

    g_ready = true;
    
    while (true) {
        this_thread::sleep_for(chrono::milliseconds(5));
    }
    stop.store(true);
    market_sim.join();
    compute_core.join();
    publisher.join();
    reader.join();
    user_changes.join();

    return 0;
}