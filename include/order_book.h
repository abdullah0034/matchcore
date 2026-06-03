#pragma once
#include <map>
#include <deque>
#include <vector>
#include <unordered_map>
#include "order.h"

namespace matchcore {

// A price level is a FIFO queue of orders resting at the same price.
struct PriceLevel {
    int64_t              price;
    std::deque<Order*>   orders;   // price-time priority: front = oldest
    int64_t              total_qty;

    PriceLevel() : price(0), total_qty(0) {}
    explicit PriceLevel(int64_t p) : price(p), total_qty(0) {}
};

// A limit order book for a single symbol. Bids are sorted highest-first,
// asks lowest-first. Matching crosses incoming orders against the opposite side.
class OrderBook {
public:
    explicit OrderBook(std::string symbol) : symbol_(std::move(symbol)) {}

    // Submit an order; returns the fills it generated. Resting remainder (if any)
    // is added to the book.
    std::vector<Fill> submit(Order* order);

    // Cancel a resting order by id. Returns true if found and removed.
    bool cancel(uint64_t order_id);

    // Best prices (0 if empty).
    int64_t best_bid() const;
    int64_t best_ask() const;

    size_t depth() const { return bids_.size() + asks_.size(); }

private:
    std::vector<Fill> match(Order* incoming);
    void rest(Order* order);

    std::string symbol_;

    // bids: highest price first.  asks: lowest price first.
    std::map<int64_t, PriceLevel, std::greater<int64_t>> bids_;
    std::map<int64_t, PriceLevel> asks_;

    // id -> (price, side) so cancel can locate the resting order.
    std::unordered_map<uint64_t, std::pair<int64_t, Side>> index_;

    uint64_t seq_ = 0;
};

}  // namespace matchcore
