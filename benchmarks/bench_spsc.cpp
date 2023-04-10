#include "hyper_buffer.hpp"
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

int main() {
    constexpr size_t Operations = 50'000'000;
    hyper::SPSCQueue<uint64_t, 65536> queue;

    auto start = std::chrono::high_resolution_clock::now();

    std::thread producer([&]() {
        for (size_t i = 1; i <= Operations; ++i) {
            while (!queue.push(i)) {
                #if defined(__x86_64__) || defined(_M_X64)
                _mm_pause();
                #endif
            }
        }
    });

    std::thread consumer([&]() {
        size_t received = 0;
        uint64_t val = 0;
        while (received < Operations) {
            if (queue.pop(val)) {
                ++received;
            } else {
                #if defined(__x86_64__) || defined(_M_X64)
                _mm_pause();
                #endif
            }
        }
    });

    producer.join();
    consumer.join();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> diff = end - start;

    std::cout << "Processed " << Operations << " ops in " << diff.count() << " seconds.\n";
    std::cout << "Throughput: " << (Operations / diff.count()) / 1e6 << " Mops/sec\n";
    return 0;
}
