#pragma once

#include "Poller.h"

#include <sys/epoll.h>

class EpollPoller : public Poller {
public:
    EpollPoller(EventLoop *eventLoop);

    ~EpollPoller() override;

    Timestamp poll(ChannelList *activeChannels) override;

    void updateChannel(Channel *channel) override;

    void removeChannel(Channel *channel) override;

private:
    void fillActiveChannels(int numEvents, ChannelList *activeChannels) const;

    void update(int operation, Channel *channel);

private:
    static const int kInitEventListSize = 16;
    using EventList = std::vector<epoll_event>;
    int m_epollFd;
    EventList m_events;
};
