// Minimal assertion-based tests for the matching engine.
// Run via: cmake --build build && ctest --test-dir build
#include <cassert>
#include <iostream>
#include "matching_engine.h"

using namespace matchcore;

static int failures = 0;
#define CHECK(cond, msg) do { if (!(cond)) { std::cerr << "FAIL: " << msg << "\n"; ++failures; } } while (0)

void test_simple_cross() {
    MatchingEngine e;
    e.add_symbol("X");
    e.submit("X", Side::Sell, OrderType::Limit, 100, 10);
    auto fills = e.submit("X", Side::Buy, OrderType::Limit, 100, 10);
    CHECK(fills.size() == 1, "expected one fill");
    CHECK(fills[0].quantity == 10, "expected qty 10");
    CHECK(fills[0].price == 100, "expected price 100");
}

void test_price_time_priority() {
    MatchingEngine e;
    e.add_symbol("X");
    // two asks at the same price — oldest should fill first
    e.submit("X", Side::Sell, OrderType::Limit, 100, 5);   // id 1, older
    e.submit("X", Side::Sell, OrderType::Limit, 100, 5);   // id 2, newer
    auto fills = e.submit("X", Side::Buy, OrderType::Limit, 100, 6);
    CHECK(fills.size() == 2, "expected to walk two resting orders");
    CHECK(fills[0].sell_order_id == 1, "oldest order should fill first");
}

int main() {
    test_simple_cross();
    test_price_time_priority();
    if (failures == 0) std::cout << "All tests passed\n";
    return failures == 0 ? 0 : 1;
}
