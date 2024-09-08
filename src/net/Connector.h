#pragma once

#include "InetAddress.h"
#include "Callbacks.h"
#include "Channel.h"

class EventLoop;

class Connector : public Channel {
public:
    using NewConnectionCallback = std::function<void(int sockFd)>;

    Connector(EventLoop *eventLoop, const InetAddress &serverAddr, int retryInterval = 500);

    ~Connector() override;

    void setNewConnectionCallback(const NewConnectionCallback &cb) { m_newConnectionCallback = cb; }

    void start();

    void restart();

    void stop();

    const InetAddress &serverAddr() const { return m_serverAddr; }

private:
    void setState(ConnectionState state) { m_state = state; }

    void startInLoop();

    void stopInLoop();

    void connect();

    void connecting(int sockFd);

    void onWrite() override;

    void onError(const std::string &errMsg) override;

    void retry(int sockFd);

    int removeChannel();

    void resetChannel();

private:
    static const int kMaxRetryIntervalMillSeconds = 30 * 1000;
    static const int kInitRetryIntervalMillSeconds = 1 * 1000;

    InetAddress m_serverAddr;
    bool m_connect;
    ConnectionState m_state;
    int m_retryIntervalMillSeconds = kInitRetryIntervalMillSeconds;
    NewConnectionCallback m_newConnectionCallback;
};
