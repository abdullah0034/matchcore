#include <iostream>
#include "matching_engine.h"

using namespace matchcore;

// Small demo: build a book, cross a few orders, print fills and stats.
int main() {
    MatchingEngine engine;
    engine.add_symbol("ACME");
    engine.set_risk_limit("ACME", 1'000'000'000);

    // Rest some asks.
    engine.submit("ACME", Side::Sell, OrderType::Limit, 101, 50);
    engine.submit("ACME", Side::Sell, OrderType::Limit, 102, 75);
    engine.submit("ACME", Side::Sell, OrderType::Limit, 100, 40);

    // An aggressive buy that should sweep the best asks.
    auto fills = engine.submit("ACME", Side::Buy, OrderType::Limit, 102, 120);

    std::cout << "Fills: " << fills.size() << "\n";
    for (const auto& f : fills) {
        std::cout << "  buy#" << f.buy_order_id << " x sell#" << f.sell_order_id
                  << "  " << f.quantity << " @ " << f.price
                  << "  (notional " << f.notional << ")\n";
    }

    auto s = engine.stats();
    std::cout << "Submitted: " << s.orders_submitted
              << "  Volume: " << s.total_volume
              << "  Notional: " << s.notional_traded << "\n";
    return 0;
}
