#include "Poller.h"

Poller::Poller(EventLoop *eventLoop) : ownerEventLoop(eventLoop) {

}

bool Poller::hasChannel(Channel *channel) const {
    const std::unordered_map<int, Channel *>::const_iterator &it = m_channels.find(channel->fd());
    return it != m_channels.end() && it->second == channel;
}

