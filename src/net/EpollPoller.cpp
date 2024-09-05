#include "EpollPoller.h"
#include "base/Logger.h"
#include "Callbacks.h"

#include <unistd.h>

EpollPoller::EpollPoller(EventLoop *eventLoop)
        : Poller(eventLoop),
          m_epollFd(epoll_create1(EPOLL_CLOEXEC)),
          m_events(kInitEventListSize) {

    if (m_epollFd < 0) {
        LOG_ERROR("epoll create failed:{}", errno);
        exit(-1);
    }
}

EpollPoller::~EpollPoller() {
    LOG_INFO("epoll disposed, fd: {} will close", m_epollFd);
    close(m_epollFd);
}

Timestamp EpollPoller::poll(Poller::ChannelList *activeChannels) {
    int numEvents = epoll_wait(m_epollFd, &*m_events.begin(), static_cast<int>(m_events.size()), -1);
    LOG_TRACE("events arrive, num of event: {}", numEvents);
    Timestamp now(Timestamp::now());
    if (numEvents > 0) {
        fillActiveChannels(numEvents, activeChannels);
    } else if (numEvents == 0) {
        LOG_TRACE("epoll wait error happens, errno: {}", errno);
    } else {
        if (errno != EINTR) { // 出现异常
            LOG_ERROR("epoll wait error: {}", errno);
        }
    }

    LOG_TRACE("event occurred time: {}", now.microSecondsSinceEpoch());
    return now;
}

void EpollPoller::updateChannel(Channel *channel) {
    int status = channel->status();
    if (status == kNew || status == kDeleted) {
        if (status == kNew) {
            int fd = channel->fd();
            m_channels[fd] = channel;
        }

        channel->setStatus(kAdded);
        update(EPOLL_CTL_ADD, channel);
    } else {
        if (channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->setStatus(kDeleted);
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EpollPoller::removeChannel(Channel *channel) {
    int fd = channel->fd();
    m_channels.erase(fd);

    int status = channel->status();
    if (status == kAdded) {
        update(EPOLL_CTL_DEL, channel);
    }

    channel->setStatus(kNew);
}

void EpollPoller::fillActiveChannels(int numEvents, Poller::ChannelList *activeChannels) const {
    for (int i = 0; i < numEvents; ++i) {
        Channel *pChannel = static_cast<Channel *>(m_events[i].data.ptr);
        int event = static_cast<int>(m_events[i].events);
        LOG_TRACE("epoll event: {}", event);
        pChannel->setREvents(event);
        activeChannels->push_back(pChannel);
    }
}

void EpollPoller::update(int operation, Channel *channel) {
    epoll_event epollEvent;
    bzero(&epollEvent, sizeof(epollEvent));

    int fd = channel->fd();
    epollEvent.events = channel->events();
    epollEvent.data.ptr = channel;

    if (epoll_ctl(m_epollFd, operation, fd, &epollEvent) < 0) {
        LOG_ERROR("epoll ctr error,operation:{},errno:{}", operation, errno);
    }
}