#pragma once

#include "Connection.h"
#include "Connector.h"

#include <mutex>

class TcpClient : public Connector {
public:
    TcpClient(EventLoop *eventLoop, const InetAddress &serverAddr, const std::string &name);

    ~TcpClient();

    void clientConnect();

    void disConnect();

    void clientStop();

    ConnectionPtr connection() const { return m_connectionPtr; }

    EventLoop *getLoop() const { return ownerLoop(); }

    bool retry() const { return m_retry; }

    void enableRetry() { m_retry = true; }

    const std::string &name() const { return m_name; }

    void setConnectionCallback(ConnectionCallback cb) { m_connectionCallback = std::move(cb); }

    void setCloseCallback(CloseCallback cb) { m_closeCallback = std::move(cb); }

    void setMessageCallback(MessageCallback cb) { m_messageCallback = std::move(cb); }

    void setWriteCompleteCallback(WriteCompleteCallback cb) { m_writeCompleteCallback = std::move(cb); }

private:
    void newConnection(int sockFd);

    void removeConnection(const ConnectionPtr &conn);

private:
    ConnectionPtr m_connectionPtr;
    const std::string m_name;
    ConnectionCallback m_connectionCallback;
    CloseCallback m_closeCallback;
    MessageCallback m_messageCallback;
    WriteCompleteCallback m_writeCompleteCallback;
    bool m_retry;
    bool m_connect;
    int m_nextConnId;
    std::mutex m_mutex;
};
