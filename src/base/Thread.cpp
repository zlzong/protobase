#include "Thread.h"
#include "CurrentThread.h"
#include "Logger.h"

#include <utility>
#include <semaphore.h>

std::atomic_int Thread::m_numCreated(0);

Thread::Thread(Thread::ThreadFunc threadFunc, std::string name)
        : m_started(false), m_joined(false),
          m_name(std::move(name)), m_tid(0),
          m_func(std::move(threadFunc)) {
    setDefaultName();
}

Thread::~Thread() {
    LOG_INFO("thread: {} disposed",m_name.c_str());
    if (m_started && !m_joined) {
        m_thread->detach();
    }
}

void Thread::start() {
    m_started = true;
    sem_t sem;
    sem_init(&sem, 0, 0);

    m_thread = std::make_shared<std::thread>([&]() {
        m_tid = CurrentThread::currentTid();
        sem_post(&sem);
        m_func();
    });

    sem_wait(&sem);
}

void Thread::join() {
    m_joined = true;
    m_thread->join();
}

void Thread::setDefaultName() {
    int num = m_numCreated++;
    if (m_name.empty()) {
        char buf[32] = {0};
        snprintf(buf, 32, "Thread%d", num);
        m_name = buf;
    }
}
