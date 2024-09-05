#include "Channel.h"
#include "base/Logger.h"
#include "EventLoop.h"


Channel::Channel(EventLoop *loop, int fd) : m_eventLoop(loop), m_fd(fd) { }

Channel::Channel(EventLoop *loop) : m_eventLoop(loop) { }

Channel::~Channel() {
    if (m_fd > 0) {
        if (int closeRet = close(m_fd) < 0) {
            LOG_ERROR("fd close ret: {}, errno: {}", closeRet, errno);
        } else {
            m_fd = -1;
        }
    }
}

void Channel::handleEvent(Timestamp receiveTime) {
    if (m_tied) {
        const std::shared_ptr<void> &guard = m_tie.lock();
        if (guard) {
            handleEventWithGuard(receiveTime);
        }
    } else {
        handleEventWithGuard(receiveTime);
    }
}

void Channel::tie(const std::shared_ptr<void> &obj) {
    m_tie = obj;
    m_tied = true;
}

void Channel::remove() {
    m_eventLoop->removeChannel(this);
}

void Channel::closeFd() {
    if (m_fd > 0) {
        int closeRet = close(m_fd);
        LOG_ERROR("fd close ret: {}, errno: {}", closeRet, errno);
        m_fd = -1;
    }
}

void Channel::update() {
    m_eventLoop->updateChannel(this);
}

void Channel::handleEventWithGuard(Timestamp receiveTime) {
    if ((m_rEvents & EPOLLHUP) && !(m_rEvents & EPOLLIN)) {
        LOG_TRACE("event EPOLLHUP happened");
        if (m_closeCallback) {
            m_closeCallback();
        }
    }

    if (m_rEvents & EPOLLERR) {
        LOG_TRACE("event EPOLLERR happened");
        if (m_errorCallback) {
            m_errorCallback("event EPOLLERR happened");
        }
    }

    if (m_rEvents & (EPOLLIN | EPOLLPRI)) {
        LOG_TRACE("event EPOLLIN or EPOLLPRI happened");
        if (m_readCallback) {
            m_readCallback(receiveTime);
        }
    }

    if (m_rEvents & EPOLLOUT) {
        LOG_TRACE("event EPOLLOUT happened");
        if (m_writeCallback) {
            m_writeCallback();
        }
    }
}
