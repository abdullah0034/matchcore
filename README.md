# MatchCore

A C++17 limit-order-matching engine — the core of a low-latency trading system.

MatchCore powers the matching layer at **Vellum Markets**, a (fictional) electronic
exchange. It maintains a price-time-priority order book per symbol, crosses
incoming orders against resting liquidity, enforces pre-trade risk limits, and
recycles order memory through an object pool to stay off the heap on the hot path.

## Architecture

```
MatchingEngine          routes orders, applies risk, owns the order pool
  └─ OrderBook (per symbol)   price-time priority, matching, resting book
       └─ PriceLevel          FIFO queue of orders at one price
  └─ RiskManager              pre-trade notional limit checks
  └─ OrderPool                recycles Order objects to avoid heap churn
```

| File | Responsibility |
|------|----------------|
| `include/order.h` | `Order`, `Fill`, enums |
| `include/order_book.h` / `src/order_book.cpp` | Book + matching logic |
| `include/matching_engine.h` / `src/matching_engine.cpp` | Top-level engine, threading, stats |
| `include/risk_manager.h` | Pre-trade risk checks |
| `include/object_pool.h` | Order memory pool |
| `src/main.cpp` | Demo |
| `tests/test_matching.cpp` | Assertion tests |

## Build & run

```bash
cmake -S . -B build
cmake --build build
./build/matchcore_demo      # demo
ctest --test-dir build      # tests
```

Requires CMake ≥ 3.16 and a C++17 compiler (g++ / clang++). The dev container
sets this up automatically.

## Domain notes

- **Prices** are integer ticks (e.g. cents) — never floating point.
- **Price-time priority:** best price first; within a price, oldest order first.
- **Notional** = price × quantity. Risk limits are per symbol.
- The engine is meant to be called concurrently from multiple market-data threads.
