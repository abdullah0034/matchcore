#pragma once
#include <unordered_map>
#include <memory>
#include <mutex>
#include <vector>
#include "order.h"
#include "order_book.h"
#include "object_pool.h"
#include "risk_manager.h"

namespace matchcore {

struct EngineStats {
    uint64_t orders_submitted;
    uint64_t orders_filled;
    uint64_t total_volume;
    // running notional traded across all symbols
    int64_t  notional_traded;
};

// Top-level engine: routes orders to per-symbol books, applies risk checks,
// recycles order memory, and is safe to call from multiple market-data threads.
class MatchingEngine {
public:
    MatchingEngine() : pool_(1 << 16) {}

    void add_symbol(const std::string& symbol);
    void set_risk_limit(const std::string& symbol, int notional_limit);

    // Submit a new order. Returns the fills generated.
    std::vector<Fill> submit(const std::string& symbol, Side side, OrderType type,
                             int64_t price, int64_t quantity);

    // Cancel a resting order across any symbol.
    bool cancel(const std::string& symbol, uint64_t order_id);

    EngineStats stats() const { return stats_; }

private:
    std::unordered_map<std::string, std::unique_ptr<OrderBook>> books_;
    std::unordered_map<std::string, std::unique_ptr<std::mutex>> locks_;
    OrderPool   pool_;
    RiskManager risk_;
    EngineStats stats_;
    uint64_t    next_id_ = 1;
};

}  // namespace matchcore
