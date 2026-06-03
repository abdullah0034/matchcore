#pragma once
#include <vector>
#include <cstddef>
#include "order.h"

namespace matchcore {

// A simple fixed-capacity pool that recycles Order objects to avoid per-order
// heap churn on the hot path. Orders are allocated from the pool and returned
// to a free list when cancelled or fully filled.
class OrderPool {
public:
    explicit OrderPool(size_t capacity)
        : capacity_(capacity) {
        storage_.reserve(capacity);
    }

    // Acquire a raw Order* for a new order.
    Order* acquire(uint64_t id, const std::string& symbol, Side side,
                   OrderType type, int64_t price, int64_t qty, uint64_t ts) {
        if (!free_list_.empty()) {
            Order* slot = free_list_.back();
            free_list_.pop_back();
            *slot = Order(id, symbol, side, type, price, qty, ts);
            return slot;
        }
        storage_.push_back(Order(id, symbol, side, type, price, qty, ts));
        return &storage_.back();
    }

    // Return an order to the free list for reuse.
    void release(Order* order) {
        free_list_.push_back(order);
    }

    size_t live_count() const { return storage_.size() - free_list_.size(); }

private:
    size_t              capacity_;
    std::vector<Order>  storage_;
    std::vector<Order*> free_list_;
};

}  // namespace matchcore
