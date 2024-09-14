#include "EventLoop.h"
#include "base/Logger.h"

#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <unistd.h>

__thread EventLoop *t_eventLoopInThisThread = nullptr;

int createEventFd() {
    int eventFd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (eventFd < 0) {
        LOG_ERROR("create evenFd failed:{}", errno);
        exit(-1);
    }

    return eventFd;
}

int createTimerFd(int64_t initialMs, int64_t intervalMs) {
    int timerFd = timerfd_create(CLOCK_MONOTONIC, EFD_NONBLOCK | EFD_CLOEXEC);
    if (timerFd < 0) {
        LOG_ERROR("create timer fd failed, errno: {}", errno);
        exit(-1);
    }

    itimerspec new_value;
    new_value.it_value.tv_sec = initialMs / 1000;
    new_value.it_value.tv_nsec = (initialMs % 1000) * 1000000;
    new_value.it_interval.tv_sec = intervalMs / 1000;
    new_value.it_interval.tv_nsec = (intervalMs % 1000) * 1000000;

    if (timerfd_settime(timerFd, 0, &new_value, nullptr) == -1) {
        LOG_ERROR("Failed to set timer, errno: {}", errno);
    }

    return timerFd;
}

EventLoop::EventLoop()
        : m_looping(false),
          m_quit(false),
          m_threadId(CurrentThread::currentTid()),
          m_poller(Poller::newDefaultPoller(this)),
          m_wakeUpChannel(new Channel(this, createEventFd())),
          m_callingPendingFunctors(false) {
    if (t_eventLoopInThisThread) {
        LOG_ERROR("another eventLoop: {} exist in this thread:{}", fmt::ptr(t_eventLoopInThisThread), m_threadId);
        exit(-1);
    } else {
        t_eventLoopInThisThread = this;
    }

    m_wakeUpChannel->setReadCallback(std::bind(&EventLoop::readEventFd, this));
    m_wakeUpChannel->enableReading();
}

EventLoop::EventLoop(std::string &name)
        : m_looping(false),
          m_quit(false),
          m_threadId(CurrentThread::currentTid()),
          m_poller(Poller::newDefaultPoller(this)),
          m_wakeUpChannel(new Channel(this, createEventFd())),
          m_callingPendingFunctors(false),
          m_name(std::move(name)) {
    if (t_eventLoopInThisThread) {
        LOG_ERROR("another eventLoop: {} exist in this thread:{}", fmt::ptr(t_eventLoopInThisThread), m_threadId);
        exit(-1);
    } else {
        t_eventLoopInThisThread = this;
    }

    m_wakeUpChannel->setReadCallback(std::bind(&EventLoop::readEventFd, this));
    m_wakeUpChannel->enableReading();
}

EventLoop::~EventLoop() {
    m_wakeUpChannel->disableAll();
    m_wakeUpChannel->remove();
    t_eventLoopInThisThread = nullptr;
}

void EventLoop::readEventFd() const {
    uint64_t one;
    ssize_t nRead = read(m_wakeUpChannel->fd(), &one, sizeof(one));
    if (nRead != sizeof(one)) {
        LOG_ERROR("read wake up fd nRead :{} is not equal to 8", nRead);
    }
}

void EventLoop::loop() {
    m_looping = true;
    m_quit = false;

    LOG_INFO("eventLoop {}:{} start looping", fmt::ptr(this), m_name.c_str());

    while (!m_quit) {
        m_activeChannels.clear();

        m_pollReturnTime = m_poller->poll(&m_activeChannels);

        for (Channel *channel: m_activeChannels) {
            channel->handleEvent(m_pollReturnTime);
        }

        doPendingFunctors();
    }

    LOG_INFO("eventLoop {} stop looping", fmt::ptr(this));
    m_looping = false;
}

void EventLoop::quit() {
    m_quit = true;

    if (!inLoopThread()) {
        wakeup();
    }
}

void EventLoop::runInLoop(EventLoop::Functor cb) {
    if (inLoopThread()) {
        cb();
    } else {
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(EventLoop::Functor cb) {
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_pendingFunctors.emplace_back(cb);
    }

    if (!inLoopThread() || m_callingPendingFunctors) {
        wakeup();
    }
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t nWrite = write(m_wakeUpChannel->fd(), &one, sizeof(one));
    if (nWrite != sizeof(one)) {
        LOG_ERROR("eventLoop wakeup error,excepted write size is {},actually write size is :{}", sizeof one, nWrite);
    }
}

void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    m_callingPendingFunctors.store(true);

    {
        std::unique_lock<std::mutex> lock(m_mutex);
        functors.swap(m_pendingFunctors);
    }

    for (const auto &functor: functors) {
        functor();
    }

    m_callingPendingFunctors.store(false);
}

void EventLoop::updateChannel(Channel *channel) {
    m_poller->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel) {
    m_poller->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel) {
    return m_poller->hasChannel(channel);
}

void EventLoop::handleTimerEvent(int tfd, TimerTask &task, Channel *channel, bool repeat) {
    uint64_t one;
    ssize_t nRead = read(tfd, &one, sizeof(one));
    if (nRead != 8) {
        LOG_ERROR("read timer fd nRead :{} is not equal to 8, errno: {}", nRead, errno);
    }

    if (inLoopThread()) {
        task();
    } else {
        runInLoop(task);
    }

    if (!repeat) {
        channel->disableAll();
        channel->remove();
        m_timerChannels.erase(channel->fd());
        delete channel;
    }
}

int EventLoop::runAt(Timestamp time, const TimerTask &task) {
    int64_t now = Timestamp::now().microSecondsSinceEpoch();
    int64_t t = time.microSecondsSinceEpoch();
    return runAfter(t-now,task);
}

int EventLoop::runAfter(int delayMs, const TimerTask &task) {
    int tfd = createTimerFd(delayMs, 0);
    auto *pChannel = new Channel(this, tfd);
    pChannel->setReadCallback(std::bind(&EventLoop::handleTimerEvent, this, tfd, task, pChannel, false));
    pChannel->enableReading();
    m_timerChannels[tfd] = pChannel;
    return tfd;
}

int EventLoop::runAfter(int delayMs, TimerTask &&task) {
    int tfd = createTimerFd(delayMs, 0);
    auto *pChannel = new Channel(this, tfd);
    pChannel->setReadCallback(std::bind(&EventLoop::handleTimerEvent, this, tfd, task, pChannel, false));
    pChannel->enableReading();
    m_timerChannels[tfd] = pChannel;
    return tfd;
}

int EventLoop::runEvery(int delayMs, const TimerTask &task) {
    int tfd = createTimerFd(delayMs, delayMs);
    auto *pChannel = new Channel(this, tfd);
    pChannel->setReadCallback(std::bind(&EventLoop::handleTimerEvent, this, tfd, task, pChannel, true));
    pChannel->enableReading();
    m_timerChannels[tfd] = pChannel;
    return tfd;
}

int EventLoop::runEvery(int delayMs, TimerTask &&task) {
    int tfd = createTimerFd(delayMs, delayMs);
    auto *pChannel = new Channel(this, tfd);
    pChannel->setReadCallback(std::bind(&EventLoop::handleTimerEvent, this, tfd, task, pChannel, true));
    pChannel->enableReading();
    m_timerChannels[tfd] = pChannel;
    return tfd;
}

void EventLoop::cancel(int timerId) {
    Channel *pChannel = m_timerChannels[timerId];
    if (pChannel) {
        pChannel->disableAll();
        pChannel->remove();
        m_timerChannels.erase(timerId);
        delete pChannel;
    }
}
