import ctypes
import os
import sys

class HyperQueue:
    """Python high-performance ctypes wrapper for native hyper::SPSCQueue."""
    def __init__(self, lib_path=None):
        if lib_path is None:
            lib_path = os.path.join(os.path.dirname(__file__), "..", "build", "libhyper_buffer.so")
        try:
            self._lib = ctypes.CDLL(lib_path)
        except OSError:
            # Fallback pure python ring buffer for environments without compiled binary
            self._lib = None
            self._buffer = [0] * 65536
            self._head = 0
            self._tail = 0
            return

        self._lib.hyper_create_u64_queue.restype = ctypes.c_void_p
        self._lib.hyper_destroy_u64_queue.argtypes = [ctypes.c_void_p]
        self._lib.hyper_push_u64.argtypes = [ctypes.c_void_p, ctypes.c_uint64]
        self._lib.hyper_push_u64.restype = ctypes.c_bool
        self._lib.hyper_pop_u64.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_uint64)]
        self._lib.hyper_pop_u64.restype = ctypes.c_bool
        self._lib.hyper_size_u64.argtypes = [ctypes.c_void_p]
        self._lib.hyper_size_u64.restype = ctypes.c_size_t

        self._handle = self._lib.hyper_create_u64_queue()

    def push(self, value: int) -> bool:
        if self._lib:
            return bool(self._lib.hyper_push_u64(self._handle, ctypes.c_uint64(value)))
        next_tail = (self._tail + 1) & 65535
        if next_tail == self._head:
            return False
        self._buffer[self._tail] = value
        self._tail = next_tail
        return True

    def pop(self) -> int:
        if self._lib:
            out = ctypes.c_uint64()
            if self._lib.hyper_pop_u64(self._handle, ctypes.byref(out)):
                return out.value
            return None
        if self._head == self._tail:
            return None
        val = self._buffer[self._head]
        self._head = (self._head + 1) & 65535
        return val

    def __del__(self):
        if hasattr(self, '_lib') and self._lib and getattr(self, '_handle', None):
            self._lib.hyper_destroy_u64_queue(self._handle)
