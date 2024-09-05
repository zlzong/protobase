#pragma once

#include "Socket.h"
#include "Channel.h"

class EventLoop;

class Acceptor : public Channel {
public:
    using NweConnectionCallback = std::function<void(int sockFd, const InetAddress &)>;

    Acceptor(EventLoop *eventLoop, const InetAddress &listenAddr);

    ~Acceptor() override;

    void setNweConnectionCallback(const NweConnectionCallback &cb);

    bool listenStatus() const;

    void listen();

private:
    void onRead(Timestamp receiveTime) override;

private:
    EventLoop *m_eventLoop;
    Socket m_socket;
    NweConnectionCallback m_newConnectionCallback;
    bool m_listening{false};
    int m_idleFd;
};
