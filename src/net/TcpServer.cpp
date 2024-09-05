#include "TcpServer.h"
#include "base/Logger.h"

TcpServer::TcpServer(EventLoop *eventLoop, const InetAddress &listenAddr, const std::string &name)
        : m_eventLoop(eventLoop),
          m_ipPort(listenAddr.getIpPort()),
          m_name(name),
          m_acceptor(new Acceptor(eventLoop, listenAddr)),
          m_eventLoopThreadPool(new EventLoopThreadPool(m_eventLoop, name)),
          m_started(0),
          m_nextConnId(1),
          m_connectionCallback(),
          m_messageCallback(),
          m_writeCompleteCallback(),
          m_threadInitCallback() {
    m_acceptor->setNweConnectionCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {
    LOG_INFO("tcp sever disposed");
    for (auto &item: m_connections) {
        ConnectionPtr connection(item.second);
        item.second.reset();
        connection->getLoop()->runInLoop(std::bind(&Connection::onConnectionDestroy, connection));
    }
}

void TcpServer::start() {
    m_eventLoopThreadPool->start(m_threadInitCallback);
    m_eventLoop->runInLoop(std::bind(&Acceptor::listen, m_acceptor.get()));
}

void TcpServer::newConnection(int connFd, const InetAddress &peerAddr) {
    EventLoop *ioLoop = m_eventLoopThreadPool->getNextLoop();
    char nameBuf[64] = {0};
    snprintf(nameBuf, 64, "-%s#%d", m_ipPort.c_str(), m_nextConnId);
    m_nextConnId++;

    std::string connName = m_name + nameBuf;
    LOG_INFO("TcpServer::newConnection [{}] -new connection [{}] from {}", m_name.c_str(), connName.c_str(), peerAddr.getIpPort().c_str());

    sockaddr_in local;
    socklen_t localAddrSize = sizeof(local);
    bzero(&local, localAddrSize);

    if (getsockname(connFd, (sockaddr *) &local, &localAddrSize) < 0) {
        LOG_ERROR("failed to get local address");
    }

    InetAddress localAddr(local);

    ConnectionPtr connection(new Connection(ioLoop, connName, connFd, localAddr, peerAddr));
    m_connections[connName] = connection;

    connection->setConnectionCallback(m_connectionCallback);
    connection->setMessageCallback(m_messageCallback);
    connection->setWriteCompleteCallback(m_writeCompleteCallback);
    connection->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));

    ioLoop->runInLoop(std::bind(&Connection::onConnectionEstablish, connection));
}

void TcpServer::removeConnection(const ConnectionPtr &conn) {
    m_eventLoop->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const ConnectionPtr &conn) {
    m_connections.erase(conn->name());
    EventLoop *pLoop = conn->getLoop();
    pLoop->queueInLoop(std::bind(&Connection::onConnectionDestroy, conn));
}
