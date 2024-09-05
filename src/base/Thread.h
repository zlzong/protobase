#pragma once

#include <memory>
#include <thread>
#include <string>
#include <atomic>
#include <functional>

class Thread {
public:
    using ThreadFunc = std::function<void()>;

    explicit Thread(ThreadFunc threadFunc, std::string name = std::string());

    ~Thread();

    void start();

    void join();

    bool started() const {
        return m_started;
    }

    pid_t tid() const {
        return m_tid;
    }

    static int numCreated() {
        return m_numCreated;
    }

private:
    void setDefaultName();

private:
    bool m_started;
    bool m_joined;
    std::string m_name;
    pid_t m_tid;
    static std::atomic_int m_numCreated;
    std::shared_ptr<std::thread> m_thread;
    ThreadFunc m_func;
};