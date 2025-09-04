#ifndef MARKET_DATA_MAILBOX
#define MARKET_DATA_MAILBOX

#define CACHELINE_SIZE 64

#include <atomic>
#include <cstdint>
#include "types.hpp"

using namespace std;

class MarketMailbox {
private:
    alignas(CACHELINE_SIZE) MarketData slots_[2];
    alignas(CACHELINE_SIZE) atomic<uint8_t> idx_;
    alignas(CACHELINE_SIZE) atomic<size_t> seq_;

public:
    MarketMailbox() : idx_(0), seq_(0) {}

    void publish(const MarketData& data) noexcept;
    const bool try_read(MarketData& out) noexcept;
    const bool read_if_updated(MarketData& out, size_t& last_seq) noexcept;
};

#endif