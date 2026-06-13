#include "order_book.h"
#include <algorithm>

namespace matchcore {

std::vector<Fill> OrderBook::submit(Order* order) {
    order->timestamp = seq_++;
    std::vector<Fill> fills = match(order);
    if (order->quantity > 0 && order->type == OrderType::Limit) {
        rest(order);
    }
    return fills;
}

std::vector<Fill> OrderBook::match(Order* incoming) {
    std::vector<Fill> fills;

    if (incoming->side == Side::Buy) {
        // Buy crosses against asks, lowest price first.
        for (auto it = asks_.begin(); it != asks_.end(); ++it) {
            PriceLevel& level = it->second;

            // Limit buy only crosses if its price >= the ask price.
            if (incoming->type == OrderType::Limit && incoming->price < level.price) {
                break;
            }

            size_t i = 0;
            while (i < level.orders.size()) {
                Order* resting = level.orders[i];
                int64_t traded = std::min(incoming->quantity, resting->quantity);

                Fill f;
                f.buy_order_id  = incoming->id;
                f.sell_order_id = resting->id;
                f.price         = resting->price;
                f.quantity      = traded;
                f.notional      = resting->price * traded;
                fills.push_back(f);

                incoming->quantity -= traded;
                resting->quantity  -= traded;
                level.total_qty    -= traded;

                if (resting->quantity == 0) {
                    resting->status = OrderStatus::Filled;
                    index_.erase(resting->id);
                    level.orders.erase(level.orders.begin() + i);
                } else {
                    ++i;
                }

                if (incoming->quantity == 0) break;
            }

            if (incoming->quantity == 0) break;
        }
    } else {
        // Sell crosses against bids, highest price first.
        for (auto it = bids_.begin(); it != bids_.end(); ++it) {
            PriceLevel& level = it->second;

            if (incoming->type == OrderType::Limit && incoming->price > level.price) {
                break;
            }

            size_t i = 0;
            while (i < level.orders.size()) {
                Order* resting = level.orders[i];
                int64_t traded = std::min(incoming->quantity, resting->quantity);

                Fill f;
                f.buy_order_id  = resting->id;
                f.sell_order_id = incoming->id;
                f.price         = resting->price;
                f.quantity      = traded;
                f.notional      = resting->price * traded;
                fills.push_back(f);

                incoming->quantity -= traded;
                resting->quantity  -= traded;
                level.total_qty    -= traded;

                if (resting->quantity == 0) {
                    resting->status = OrderStatus::Filled;
                    index_.erase(resting->id);
                    level.orders.erase(level.orders.begin() + i);
                } else {
                    ++i;
                }

                if (incoming->quantity == 0) break;
            }

            if (incoming->quantity == 0) break;
        }
    }

    if (incoming->quantity < incoming->original_quantity && incoming->quantity > 0) {
        incoming->status = OrderStatus::PartiallyFilled;
    } else if (incoming->quantity == 0) {
        incoming->status = OrderStatus::Filled;
    }

    return fills;
}

void OrderBook::rest(Order* order) {
    if (order->side == Side::Buy) {
        PriceLevel& level = bids_[order->price];
        level.price = order->price;
        level.orders.push_back(order);
        level.total_qty += order->quantity;
    } else {
        PriceLevel& level = asks_[order->price];
        level.price = order->price;
        level.orders.push_back(order);
        level.total_qty += order->quantity;
    }
    index_[order->id] = { order->price, order->side };
}

bool OrderBook::cancel(uint64_t order_id) {
    auto idx = index_.find(order_id);
    if (idx == index_.end()) return false;

    int64_t price = idx->second.first;
    Side side     = idx->second.second;

    if (side == Side::Buy) {
        PriceLevel& level = bids_[price];
        for (size_t i = 0; i < level.orders.size(); ++i) {
            if (level.orders[i]->id == order_id) {
                level.total_qty -= level.orders[i]->quantity;
                level.orders.erase(level.orders.begin() + i);
                break;
            }
        }
        if (level.orders.empty()) bids_.erase(price);
    } else {
        PriceLevel& level = asks_[price];
        for (size_t i = 0; i < level.orders.size(); ++i) {
            if (level.orders[i]->id == order_id) {
                level.total_qty -= level.orders[i]->quantity;
                level.orders.erase(level.orders.begin() + i);
                break;
            }
        }
        if (level.orders.empty()) asks_.erase(price);
    }

    index_.erase(order_id);
    return true;
}

int64_t OrderBook::best_bid() const {
    if (bids_.empty()) return 0;
    return bids_.begin()->first;
}

int64_t OrderBook::best_ask() const {
    if (asks_.empty()) return 0;
    return asks_.begin()->first;
}

}  // namespace matchcore
