#include <unordered_map>
#include <exception>
#include <chrono>
#include <iostream>
#include <string>
#include <string_view>
#include <sw/redis++/redis++.h>
#include <boost/lockfree/spsc_queue.hpp>
#include "redis_publisher.hpp"
#include "utils.hpp"

using namespace std;
using namespace sw::redis;


RedisPublisher::RedisPublisher(string_view host, size_t port)
    : redis_([&] {
        ConnectionOptions opts;
        opts.host = string(host);
        opts.port = static_cast<int>(port);
        return opts;
    }()),
    pipeline_(redis_.pipeline()),
    jobs_queue_(512)
{
    greeks_hset_ = {
        {"as_of"                , ""},
        {"spot"                 , ""},
        {"vol"                  , ""},
        {"rate"                 , ""},
        {"div_yield"            , ""},
        {"theo_price"           , ""},
        {"delta"                , ""},
        {"gamma"                , ""},
        {"vega"                 , ""},
        {"rho"                  , ""},
        {"theta"                , ""},
        {"calibration_version"  , ""},
        {"seqno"                , ""}
    };
    run();
}

void RedisPublisher::run() {
    PublishJob cur_job;
    if (jobs_queue_.pop(cur_job)) {
        BatchKey job_key = {cur_job.symbol, cur_job.expiry, cur_job.engine_type};
        if (cur_job.seqno > latest_seqno_[job_key] || !latest_seqno_.contains(job_key)) {
            latest_seqno_[job_key] = cur_job.seqno;
            publish_batch(cur_job);
        }
    } else {
        this_thread::sleep_for(chrono::milliseconds(10));
    }
}

void RedisPublisher::publish_batch(const PublishJob& job) {
    size_t len = job.strikes->size();
    assert(job.payoff_types->size() == len);
    assert(job.ranges->size() > 0);
    assert(job.theo.size() == len);
    assert(job.delta.size() == len);
    assert(job.gamma.size() == len);
    assert(job.vega.size() == len);
    assert(job.rho.size() == len);
    assert(job.theta.size() == len);

    greeks_hset_["as_of"]                   = ns_to_iso8601(job.as_of_ns);
    greeks_hset_["spot"]                    = to_string(job.spot);
    greeks_hset_["vol"]                     = to_string(job.vol);
    greeks_hset_["rate"]                    = to_string(job.rate);
    greeks_hset_["div_yield"]               = to_string(job.div_yield);
    greeks_hset_["calibration_version"]     = to_string(job.calibration_version);
    greeks_hset_["seqno"]                   = to_string(job.seqno);

    for (auto& range : *job.ranges) {
        for (size_t i = range.first; i < range.second; i++) {
            if (i >= len) {
                break;
            }
            
            char strike_buf[32];
            snprintf(strike_buf, sizeof(strike_buf), "%.4f", job.strikes->at(i));
            string key = (
                "greeks:"
                + job.symbol + ":"
                + ns_to_date(job.expiry) + ":"
                + strike_buf + ":"
                + payoff_type_to_string(job.payoff_types->at(i))
            );
            // cout << "redis key: " << key << '\n';
            greeks_hset_["theo_price"]              = to_string(job.theo.at(i));
            greeks_hset_["delta"]                   = to_string(job.delta.at(i));
            greeks_hset_["gamma"]                   = to_string(job.gamma.at(i));
            greeks_hset_["vega"]                    = to_string(job.vega.at(i));
            greeks_hset_["rho"]                     = to_string(job.rho.at(i));
            greeks_hset_["theta"]                   = to_string(job.theta.at(i));

            string_view key_sv{key};
            pipeline_.hmset(key_sv, greeks_hset_.begin(), greeks_hset_.end());
            pipeline_.expire(key_sv, ttl_seconds_);
        }
    }
    try {
        auto replies = pipeline_.exec();
    } catch (const exception& e) {
        cout << "error: " << e.what() << '\n';
        pipeline_ = redis_.pipeline();
    }
}

void RedisPublisher::enqueue_job(PublishJob job) {
    if (jobs_queue_.write_available()) {
        jobs_queue_.push(std::move(job));
    } else {
        //TODO: implement
    }
}