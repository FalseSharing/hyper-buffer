cmake_minimum_required(VERSION 3.20)
project(hyper_buffer VERSION 1.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include_directories(include)

add_library(hyper_buffer SHARED src/hyper_buffer.cpp)
target_include_directories(hyper_buffer PUBLIC include)

add_executable(bench_throughput benchmarks/bench_spsc.cpp)
target_link_libraries(bench_throughput PRIVATE hyper_buffer)
