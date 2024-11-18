#pragma once

#include "base/Thread.h"

#include <functional>
#include <string>
#include <mutex>
#include <condition_variable>

class EventLoop;

class EventLoopThread {
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;

    EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(), const std::string &name = std::string());

    ~EventLoopThread();

    EventLoop *startLoop();

    EventLoop *getLoop() const {
        return m_eventLoop;
    }

private:
    void threadFunc();

    EventLoop *m_eventLoop;
    bool m_exiting;
    Thread m_thread;
    std::mutex m_mutex;
    std::condition_variable m_condition;
    ThreadInitCallback m_threadInit;
};
