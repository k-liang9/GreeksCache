#include <atomic>
#include <cstdint>
#include "mailbox.hpp"

void MarketMailbox::publish(const MarketData& data) noexcept {
    const uint8_t w = 1u - idx_.load(memory_order_relaxed);
    slots_[w] = data;
    idx_.store(w, memory_order_release);
    seq_.fetch_add(1, memory_order_release);
}

const bool MarketMailbox::try_read(MarketData& out) noexcept {
    const uint8_t r1 = idx_.load(memory_order_acquire);
    out = slots_[r1];
    const uint8_t r2 = idx_.load(memory_order_acquire);
    return r1 == r2;
}

const bool MarketMailbox::read_if_updated(MarketData& out, size_t& last_seq) noexcept {
    const size_t s1 = seq_.load(memory_order_acquire);
    if (s1 == last_seq) return false;
    for (size_t i = 0; i < 3; i++) {
        if (try_read(out)) {
            last_seq = s1;
            return true;
        }
    }
    
    (void)try_read(out);
    last_seq = seq_.load(memory_order_acquire);
    return true;
}