#pragma once

#include "base/Timestamp.h"
#include "base/CurrentThread.h"
#include "Poller.h"
#include "TimerWheel.h"

#include <vector>
#include <atomic>
#include <memory>
#include <functional>
#include <mutex>

class Channel;

class EventLoop {
public:
    using Functor = std::function<void()>;

    EventLoop();

    explicit EventLoop(std::string &name);

    ~EventLoop();

    void loop();

    void quit();

    Timestamp pollReturnTime() const { return m_pollReturnTime; }

    void runInLoop(Functor cb);

    void queueInLoop(Functor cb);

    void wakeup();

    void updateChannel(Channel *channel);

    void removeChannel(Channel *channel);

    bool hasChannel(Channel *channel);

    bool inLoopThread() const { return m_threadId == CurrentThread::currentTid(); }

    int runAt(Timestamp time, const TimerTask &task);

    int runAfter(int delayMs, const TimerTask &task);

    int runAfter(int delayMs, TimerTask &&task);

    int runEvery(int intervalMs, const TimerTask &task);

    int runEvery(int intervalMs, TimerTask &&task);

    void cancel(int timerId);

private:
    void handleTimerEvent(int tFd, TimerTask &task, Channel *channel, bool repeat);

    void readEventFd() const;

    void doPendingFunctors();

private:
    using ChannelList = std::vector<Channel *>;

    std::atomic<bool> m_looping;
    std::atomic<bool> m_quit;
    const pid_t m_threadId;
    Timestamp m_pollReturnTime;
    std::unique_ptr<Poller> m_poller;
    std::unique_ptr<Channel> m_wakeUpChannel;
    ChannelList m_activeChannels;
    std::atomic<bool> m_callingPendingFunctors;
    std::vector<Functor> m_pendingFunctors;
    std::mutex m_mutex;
    std::string m_name;
    std::unordered_map<int,Channel *> m_timerChannels;
};
