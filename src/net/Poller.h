#pragma once

#include "Channel.h"

#include <vector>
#include <unordered_map>

class Poller {
public:
    using ChannelList = std::vector<Channel *>;

    explicit Poller(EventLoop *eventLoop);

    virtual ~Poller() = default;

    virtual Timestamp poll(ChannelList *activeChannels) = 0;

    virtual void updateChannel(Channel *channel) = 0;

    virtual void removeChannel(Channel *channel) = 0;

    bool hasChannel(Channel *channel) const;

    static Poller *newDefaultPoller(EventLoop *eventLoop);

protected:
    using ChannelMap = std::unordered_map<int, Channel *>;
    ChannelMap m_channels;
private:
    EventLoop *ownerEventLoop;
};