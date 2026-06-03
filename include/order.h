#pragma once
#include <cstdint>
#include <string>

namespace matchcore {

enum class Side { Buy, Sell };
enum class OrderType { Limit, Market };
enum class OrderStatus { New, PartiallyFilled, Filled, Cancelled };

// A single resting or incoming order.
struct Order {
    uint64_t    id;
    std::string symbol;
    Side        side;
    OrderType   type;
    int64_t     price;      // price in integer ticks (e.g. cents)
    int64_t     quantity;   // remaining quantity
    int64_t     original_quantity;
    uint64_t    timestamp;  // arrival sequence for price-time priority
    OrderStatus status;

    Order(uint64_t id_, std::string symbol_, Side side_, OrderType type_,
          int64_t price_, int64_t quantity_, uint64_t ts_)
        : id(id_), symbol(std::move(symbol_)), side(side_), type(type_),
          price(price_), quantity(quantity_), original_quantity(quantity_),
          timestamp(ts_), status(OrderStatus::New) {}

    bool is_filled() const { return quantity == 0; }
};

// Reports a single match (a trade) between two orders.
struct Fill {
    uint64_t buy_order_id;
    uint64_t sell_order_id;
    int64_t  price;
    int64_t  quantity;
    int64_t  notional;   // price * quantity
};

}  // namespace matchcore
