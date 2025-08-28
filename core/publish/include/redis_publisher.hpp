#ifndef REDIS_PUBLISHER
#define REDIS_PUBLISHER

#include <unordered_map>
#include <chrono>
#include <sw/redis++/redis++.h>
#include <boost/lockfree/spsc_queue.hpp>
#include "types.hpp"

using namespace std;
using namespace sw::redis;
using namespace boost::lockfree;

struct BatchKey{
    string symbol;
    t_ns expiry_date;
    EngineType engine_type;
    
    bool operator==(const BatchKey& other) const {
        return symbol == other.symbol && 
               expiry_date == other.expiry_date && 
               engine_type == other.engine_type;
    }
};

struct BatchKeyHash {
    size_t operator()(const BatchKey& key) const {
        size_t h1 = hash<string>{}(key.symbol);
        size_t h2 = hash<t_ns>{}(key.expiry_date);
        size_t h3 = hash<int>{}(static_cast<int>(key.engine_type));
        
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};



class RedisPublisher {
private:
    Redis redis_;
    Pipeline pipeline_;
    spsc_queue<PublishJob> jobs_queue_;
    unordered_map<BatchKey, size_t, BatchKeyHash> latest_seqno_;
    unordered_map<string, string> greeks_hset_;
    chrono::seconds ttl_seconds_ = chrono::seconds(5);

    void publish_batch(const PublishJob& job);
public:
    RedisPublisher(string_view host, size_t port);
    void run();
    void enqueue_job(PublishJob job);
};

#endif