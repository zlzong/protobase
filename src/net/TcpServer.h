#pragma once

#include "Callbacks.h"
#include "InetAddress.h"
#include "Connection.h"
#include "EventLoop.h"
#include "Acceptor.h"
#include "EventLoopThreadPool.h"

#include <unordered_map>
#include <atomic>

class TcpServer {
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;

    TcpServer(EventLoop *eventLoop, const InetAddress &listenAddr, const std::string &name);

    ~TcpServer();

    void setThreadInitCallback(const ThreadInitCallback &cb) { m_threadInitCallback = cb; }

    void setConnectionCallback(const ConnectionCallback &cb) { m_connectionCallback = cb; }

    void setMessageCallback(const MessageCallback &cb) { m_messageCallback = cb; }

    void setWriteCompleteCallback(const WriteCompleteCallback &cb) { m_writeCompleteCallback = cb; }

    void setThreadNum(int numThreads) { m_eventLoopThreadPool->setThreadNum(numThreads); }

    void start();

private:
    using ConnectionMap = std::unordered_map<std::string, ConnectionPtr>;
    EventLoop *m_eventLoop;
    const std::string m_ipPort;
    const std::string m_name;
    std::unique_ptr<Acceptor> m_acceptor;
    std::shared_ptr<EventLoopThreadPool> m_eventLoopThreadPool;
    std::atomic<int> m_started;
    int m_nextConnId;
    ConnectionMap m_connections;
    ConnectionCallback m_connectionCallback;
    MessageCallback m_messageCallback;
    WriteCompleteCallback m_writeCompleteCallback;
    ThreadInitCallback m_threadInitCallback;
private:
    void newConnection(int connFd, const InetAddress &peerAddr);

    void removeConnection(const ConnectionPtr &conn);

    void removeConnectionInLoop(const ConnectionPtr &conn);
};
