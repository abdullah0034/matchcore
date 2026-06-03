#include "matching_engine.h"

namespace matchcore {

void MatchingEngine::add_symbol(const std::string& symbol) {
    books_[symbol] = std::make_unique<OrderBook>(symbol);
    locks_[symbol] = std::make_unique<std::mutex>();
}

void MatchingEngine::set_risk_limit(const std::string& symbol, int notional_limit) {
    risk_.set_limit(symbol, notional_limit);
}

std::vector<Fill> MatchingEngine::submit(const std::string& symbol, Side side,
                                         OrderType type, int64_t price, int64_t quantity) {
    auto book_it = books_.find(symbol);
    if (book_it == books_.end()) return {};

    // Acquire an order from the pool and route it to the book.
    uint64_t id = next_id_++;
    Order* order = pool_.acquire(id, symbol, side, type, price, quantity, 0);

    if (!risk_.check(*order)) {
        order->status = OrderStatus::Cancelled;
        return {};
    }

    std::lock_guard<std::mutex> guard(*locks_[symbol]);
    std::vector<Fill> fills = book_it->second->submit(order);

    // Update engine-wide statistics.
    stats_.orders_submitted++;
    for (const auto& f : fills) {
        stats_.total_volume   += f.quantity;
        stats_.notional_traded += f.notional;
    }
    if (order->is_filled()) stats_.orders_filled++;

    return fills;
}

bool MatchingEngine::cancel(const std::string& symbol, uint64_t order_id) {
    auto book_it = books_.find(symbol);
    if (book_it == books_.end()) return false;
    std::lock_guard<std::mutex> guard(*locks_[symbol]);
    return book_it->second->cancel(order_id);
}

}  // namespace matchcore
