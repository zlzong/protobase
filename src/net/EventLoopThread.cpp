#include "EventLoopThread.h"
#include "EventLoop.h"

EventLoopThread::EventLoopThread(const EventLoopThread::ThreadInitCallback &cb, const std::string &name)
        : m_eventLoop(nullptr),
          m_exiting(false),
          m_thread(std::bind(&EventLoopThread::threadFunc, this), name),
          m_condition(),
          m_threadInit(cb) {

}

EventLoopThread::~EventLoopThread() {
    m_exiting = true;
    if (m_eventLoop != nullptr) {
        m_eventLoop->quit();
        m_thread.join();
    }
}

EventLoop *EventLoopThread::startLoop() {
    m_thread.start();
    EventLoop *loop = nullptr;

    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (!m_eventLoop) {
            m_condition.wait(lock);
        }

        loop = m_eventLoop;
    }

    return loop;
}

void EventLoopThread::threadFunc() {
    EventLoop loop;

    if (m_threadInit) {
        m_threadInit(&loop);
    }

    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_eventLoop = &loop;
        m_condition.notify_one();
    }

    loop.loop();

    std::unique_lock<std::mutex> lock(m_mutex);
    m_eventLoop = nullptr;
}
