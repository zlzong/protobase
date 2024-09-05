#pragma once

#include "EventLoopThread.h"

#include <functional>
#include <string>
#include <vector>
#include <memory>

class EventLoop;

class EventLoopThreadPool {
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;

    EventLoopThreadPool(EventLoop *baseEventLoop, const std::string &name, int threadNum = 0);

    ~EventLoopThreadPool() = default;

    void setThreadNum(int numThreads) { m_numThreads = numThreads; }

    void start(const ThreadInitCallback &cb = ThreadInitCallback());

    EventLoop *getNextLoop();

    bool started() const { return m_started; }

    const std::string name() const { return m_name; }

private:
    EventLoop *m_baseEventLoop;
    std::string m_name;
    bool m_started;
    int m_numThreads;
    int m_next;
    std::vector<std::unique_ptr<EventLoopThread>> m_threads;
    std::vector<EventLoop *> m_loops;
};
