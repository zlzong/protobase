#include "EventLoopThreadPool.h"
#include "base/Logger.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseEventLoop, const std::string &name, int threadNum)
        : m_baseEventLoop(baseEventLoop),
          m_name(name),
          m_started(false),
          m_numThreads(threadNum),
          m_next(0) {

}

void EventLoopThreadPool::start(const EventLoopThreadPool::ThreadInitCallback &cb) {
    m_started = true;

    for (int i = 0; i < m_numThreads; ++i) {
        char threadName[m_name.size() + 32];
        snprintf(threadName, sizeof(threadName), "%s%d", m_name.c_str(), i);
        EventLoopThread *pThread = new EventLoopThread(cb, threadName);
        m_threads.push_back(std::unique_ptr<EventLoopThread>(pThread));
        m_loops.push_back(pThread->startLoop());
    }

    if (m_numThreads == 0 && cb) {
        cb(m_baseEventLoop);
    }
}

EventLoop *EventLoopThreadPool::getNextLoop() {
    EventLoop *loop = m_baseEventLoop;

    if (!m_loops.empty()) {
        loop = m_loops[m_next];

        LOG_DEBUG("loop address: {}", fmt::ptr(loop));
        m_next++;

        if (m_next >= m_loops.size()) {
            m_next = 0;
        }
    }

    return loop;
}