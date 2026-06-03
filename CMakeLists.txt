cmake_minimum_required(VERSION 3.16)
project(MatchCore CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

add_compile_options(-Wall -Wextra -g)

include_directories(${CMAKE_SOURCE_DIR}/include)

find_package(Threads REQUIRED)

add_library(matchcore
  src/order_book.cpp
  src/matching_engine.cpp
)
target_link_libraries(matchcore PUBLIC Threads::Threads)

add_executable(matchcore_demo src/main.cpp)
target_link_libraries(matchcore_demo PRIVATE matchcore)

enable_testing()
add_executable(matchcore_tests tests/test_matching.cpp)
target_link_libraries(matchcore_tests PRIVATE matchcore)
add_test(NAME matching COMMAND matchcore_tests)
