#include "Poller.h"
#include "EpollPoller.h"

Poller *Poller::newDefaultPoller(EventLoop *eventLoop) {
    if (std::getenv("USE_POLL")) {
        // todo 生成poll的实例
        return nullptr;
    } else {
        return new EpollPoller(eventLoop);
    }
}