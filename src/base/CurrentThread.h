#pragma once

namespace CurrentThread {
    extern __thread int cachedThreadId;

    void cachedTid();

    inline int currentTid() {
        if (cachedThreadId == 0) {
            cachedTid();
        }
        return cachedThreadId;
    }
}
