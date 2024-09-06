#pragma once

#include "base/Timestamp.h"

#include <memory>
#include <functional>
#include <sys/epoll.h>

const int kNoneEvent = 0;
const int kReadEvent = EPOLLIN | EPOLLPRI;
const int kWriteEvent = EPOLLOUT;

class EventLoop;

class Channel : public std::enable_shared_from_this<Channel> {
public:
    using ErrorCallback = std::function<void(const std::string &)>;
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop *loop, int fd);

    explicit Channel(EventLoop *loop);

    virtual ~Channel();

    virtual void onRead(Timestamp receiveTime) {};

    virtual void onWrite() {};

    virtual void onClose() {};

    virtual void onError(const std::string &errMsg) {};

    void setFd(int fd) {m_fd = fd;};

    void handleEvent(Timestamp receiveTime);

    void setReadCallback(ReadEventCallback cb) {
        m_readCallback = std::move(cb);
    }

    void setWriteCallback(EventCallback cb) {
        m_writeCallback = std::move(cb);
    }

    void setCloseCallback(EventCallback cb) {
        m_closeCallback = std::move(cb);
    }

    void setErrorCallback(ErrorCallback cb) {
        m_errorCallback = std::move(cb);
    }

    void tie(const std::shared_ptr<void> &obj);

    int fd() const {
        return m_fd;
    }

    int events() const {
        return m_events;
    }

    void setREvents(int rEvent) {
        m_rEvents = rEvent;
    }

    void enableReading() {
        m_events |= kReadEvent;
        update();
    }

    void disableReading() {
        m_events &= ~kReadEvent;
        update();
    }

    void enableWriting() {
        m_events |= kWriteEvent;
        update();
    }

    void disableWriting() {
        m_events &= ~kWriteEvent;
        update();
    }

    void disableAll() {
        m_events = kNoneEvent;
        update();
    }

    bool isNoneEvent() const {
        return m_events == kNoneEvent;
    }

    bool isWriting() const {
        return m_events & kWriteEvent;
    }

    bool isReading() const {
        return m_events & kReadEvent;
    }

    int status() const {
        return m_status;
    }

    void setStatus(int status) {
        m_status = status;
    }

    EventLoop *ownerLoop() const {
        return m_eventLoop;
    }

    void remove();

    void closeFd();

private:
    void update();

    void handleEventWithGuard(Timestamp receiveTime);

private:
    EventLoop *m_eventLoop;
    int m_fd{-1};
    int m_events{0};
    int m_rEvents{0};
    int m_status{-1};
    bool m_tied{false};
    std::weak_ptr<void> m_tie;
    ReadEventCallback m_readCallback;
    EventCallback m_writeCallback;
    EventCallback m_closeCallback;
    ErrorCallback m_errorCallback;
};