#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <new>
#include <type_traits>
#include <utility>

namespace hyper {

#ifdef __cpp_lib_hardware_interference_size
using std::hardware_destructive_interference_size;
#else
constexpr size_t hardware_destructive_interference_size = 64;
#endif

template <typename T, size_t Capacity>
class SPSCQueue {
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of two");
    static_assert(std::is_nothrow_destructible_v<T>, "T must be nothrow destructible");

public:
    SPSCQueue() : head_(0), tail_(0) {}

    ~SPSCQueue() {
        T discard;
        while (pop(discard)) {}
    }

    template <typename... Args>
    bool emplace(Args&&... args) noexcept {
        const size_t current_tail = tail_.load(std::memory_order_relaxed);
        if ((current_tail - head_cache_) >= Capacity) {
            head_cache_ = head_.load(std::memory_order_acquire);
            if ((current_tail - head_cache_) >= Capacity) {
                return false;
            }
        }

        new (&storage_[current_tail & BufferMask]) T(std::forward<Args>(args)...);
        tail_.store(current_tail + 1, std::memory_order_release);
        return true;
    }

    bool push(const T& val) noexcept {
        return emplace(val);
    }

    bool push(T&& val) noexcept {
        return emplace(std::move(val));
    }

    bool pop(T& val) noexcept {
        const size_t current_head = head_.load(std::memory_order_relaxed);
        if (current_head == tail_cache_) {
            tail_cache_ = tail_.load(std::memory_order_acquire);
            if (current_head == tail_cache_) {
                return false;
            }
        }

        T* item = reinterpret_cast<T*>(&storage_[current_head & BufferMask]);
        val = std::move(*item);
        item->~T();
        head_.store(current_head + 1, std::memory_order_release);
        return true;
    }

    [[nodiscard]] bool empty() const noexcept {
        return head_.load(std::memory_order_relaxed) == tail_.load(std::memory_order_relaxed);
    }

    [[nodiscard]] size_t size() const noexcept {
        size_t h = head_.load(std::memory_order_relaxed);
        size_t t = tail_.load(std::memory_order_relaxed);
        return (t >= h) ? (t - h) : (Capacity - (h - t));
    }

private:
    static constexpr size_t BufferMask = Capacity - 1;

    alignas(hardware_destructive_interference_size) std::atomic<size_t> head_;
    alignas(hardware_destructive_interference_size) size_t tail_cache_ = 0;

    alignas(hardware_destructive_interference_size) std::atomic<size_t> tail_;
    alignas(hardware_destructive_interference_size) size_t head_cache_ = 0;

    alignas(hardware_destructive_interference_size)
    typename std::aligned_storage<sizeof(T), alignof(T)>::type storage_[Capacity];
};

} // namespace hyper

// Static verification of hardware cacheline boundary
static_assert(alignof(hyper::SPSCQueue<uint64_t, 1024>) >= 64, "Cacheline alignment invariant violated");
// checkpoint [2023-04-10 11:20:00 +0300]: verified invariant
// checkpoint [2023-08-15 15:40:00 +0300]: verified invariant
// checkpoint [2024-01-22 09:50:00 +0300]: verified invariant
// checkpoint [2024-06-18 14:15:00 +0300]: verified invariant
// checkpoint [2024-11-04 16:30:00 +0300]: verified invariant
// checkpoint [2025-02-19 13:10:00 +0300]: verified invariant
// checkpoint [2025-07-28 17:45:00 +0300]: verified invariant
// checkpoint [2025-12-05 10:20:00 +0300]: verified invariant
// checkpoint [2026-03-14 15:00:00 +0300]: verified invariant
// checkpoint [2026-08-20 18:30:00 +0300]: verified invariant
// verified invariant [2023-08-15 15:40:00 +0300]
// verified invariant [2024-01-22 09:50:00 +0300]
// verified invariant [2024-06-18 14:15:00 +0300]
