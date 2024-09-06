#include "TcpClient.h"
#include "Connector.h"
#include "base/Logger.h"
#include "EventLoop.h"

// why?
namespace detail {
    void removeConnection(EventLoop *loop, const ConnectionPtr &conn) {
        loop->queueInLoop(std::bind(&Connection::onConnectionDestroy, conn));
    }
}

TcpClient::TcpClient(EventLoop *eventLoop, const InetAddress &serverAddr, const std::string &name)
        : Connector(eventLoop, serverAddr), m_eventLoop(eventLoop), m_name(name), m_retry(false), m_connect(false), m_nextConnId(1) {
    setNewConnectionCallback(std::bind(&TcpClient::newConnection, this, std::placeholders::_1));
}

TcpClient::~TcpClient() {
    ConnectionPtr conn;
    bool unique = false;

    {
        std::unique_lock<std::mutex> lk(m_mutex);
        unique = m_connectionPtr.unique();
        conn = m_connectionPtr;
    }

    if (conn) {
        CloseCallback cb = std::bind(&detail::removeConnection, m_eventLoop, std::placeholders::_1);
        m_eventLoop->runInLoop(std::bind(&Connection::setCloseCallback, conn, cb));
        if (unique) {
            conn->forceClose();
        }
    } else {
        stop();
    }
}

void TcpClient::clientConnect() {
    m_connect = true;
    start();
}

void TcpClient::disConnect() {
    m_connect = false;

    {
        std::unique_lock<std::mutex> lk(m_mutex);
        if (m_connectionPtr) {
            m_connectionPtr->shutdown();
        }
    }
}

void TcpClient::clientStop() {
    m_connect = false;
    stop();
}

void TcpClient::newConnection(int sockFd) {
    LOG_INFO("new connection arrive: sockFd is: {}", sockFd);
    sockaddr_in peer;
    socklen_t peerAddrLen = sizeof(peer);
    ::getpeername(sockFd, (sockaddr *) &peer, &peerAddrLen);
    InetAddress peerAddr(peer);

    sockaddr_in local;
    socklen_t localAddrLen = sizeof(local);
    ::getsockname(sockFd, (sockaddr *) &local, &localAddrLen);
    InetAddress localAddr(peer);

    char buf[32];
    snprintf(buf, sizeof buf, ":%s#%d", peerAddr.getIpPort().c_str(), m_nextConnId);
    m_nextConnId++;
    std::string connName = m_name + buf;

    ConnectionPtr conn(new Connection(m_eventLoop, connName, sockFd, localAddr, peerAddr));
    conn->setConnectionCallback(m_connectionCallback);
    conn->setMessageCallback(m_messageCallback);
    conn->setWriteCompleteCallback(m_writeCompleteCallback);
    conn->setCloseCallback(std::bind(&TcpClient::removeConnection, this, std::placeholders::_1));

    {
        std::unique_lock<std::mutex> lk(m_mutex);
        m_connectionPtr = conn;
    }

    conn->onConnectionEstablish();
}

void TcpClient::removeConnection(const ConnectionPtr &conn) {
    {
        std::unique_lock<std::mutex> lk(m_mutex);
        m_connectionPtr.reset();
    }

    m_eventLoop->queueInLoop(std::bind(&Connection::onConnectionDestroy, conn));
    if (m_closeCallback) {
        m_closeCallback(conn);
    }

    if (m_retry && m_connect) {
        restart();
    }
}
