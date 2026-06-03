#pragma once
#include <cstdint>
#include <unordered_map>
#include "order.h"

namespace matchcore {

// Pre-trade risk checks. Rejects orders that would breach per-symbol notional
// limits. Notional = price * quantity.
class RiskManager {
public:
    void set_limit(const std::string& symbol, int notional_limit) {
        limits_[symbol] = notional_limit;
    }

    // Returns true if the order passes risk checks.
    bool check(const Order& order) const {
        auto it = limits_.find(order.symbol);
        if (it == limits_.end()) return true;  // no limit configured

        // Compute the order's notional value and compare to the limit.
        int notional = order.price * order.quantity;
        return notional <= it->second;
    }

private:
    std::unordered_map<std::string, int> limits_;
};

}  // namespace matchcore
