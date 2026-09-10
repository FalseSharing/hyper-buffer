#include "hyper_buffer.hpp"
#include <iostream>

extern "C" {

struct ByteBufferHandle;

ByteBufferHandle* hyper_create_u64_queue() {
    auto* q = new hyper::SPSCQueue<uint64_t, 65536>();
    return reinterpret_cast<ByteBufferHandle*>(q);
}

void hyper_destroy_u64_queue(ByteBufferHandle* handle) {
    auto* q = reinterpret_cast<hyper::SPSCQueue<uint64_t, 65536>*>(handle);
    delete q;
}

bool hyper_push_u64(ByteBufferHandle* handle, uint64_t val) {
    auto* q = reinterpret_cast<hyper::SPSCQueue<uint64_t, 65536>*>(handle);
    return q->push(val);
}

bool hyper_pop_u64(ByteBufferHandle* handle, uint64_t* val_out) {
    auto* q = reinterpret_cast<hyper::SPSCQueue<uint64_t, 65536>*>(handle);
    return q->pop(*val_out);
}

size_t hyper_size_u64(ByteBufferHandle* handle) {
    auto* q = reinterpret_cast<hyper::SPSCQueue<uint64_t, 65536>*>(handle);
    return q->size();
}

}
