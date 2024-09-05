#include "CurrentThread.h"

#include <syscall.h>
#include <unistd.h>

namespace CurrentThread {
    __thread int cachedThreadId = 0;

    void cachedTid() {
        if (cachedThreadId == 0) {
            cachedThreadId = static_cast<pid_t>(::syscall(SYS_gettid));
        }
    }
}