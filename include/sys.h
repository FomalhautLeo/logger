#pragma once

namespace logger {

namespace memory {

    inline void fence() {
        asm volatile("mfence" : : : "memory");
    }

    inline void rfence() {
        asm volatile("lfence" : : : "memory");
    }

    inline void wfence() {
        asm volatile("sfence" : : : "memory");
    }

}  // namespace memory

}  // namespace logger