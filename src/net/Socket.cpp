#include "Socket.h"
#include "InetAddress.h"
#include "base/Logger.h"

#include <netinet/tcp.h>

Socket::Socket(int sockFd) : m_sockFd(sockFd) { }

int Socket::fd() const {
    return m_sockFd;
}

void Socket::bindAddress(const InetAddress &localAddr) const {
    LOG_INFO("Socket::bindAddress called, localAddr: {}", localAddr.getIpPort().c_str());
    int bindRet = bind(m_sockFd, (sockaddr *) localAddr.getSockAddrIn(), sizeof(sockaddr_in));
    LOG_TRACE("bind ret: {}", bindRet);
    if (bindRet != 0) {
        LOG_ERROR("socket bind error,bind ret: {},errno: {}", bindRet, errno);
        exit(-1);
    }
}

void Socket::listen() {
    int listenRet = ::listen(m_sockFd, 1024);
    LOG_TRACE("listen ret: {}", listenRet);
    if (listenRet != 0) {
        LOG_ERROR("socket listen error,bind ret: {},errno: {}", listenRet, errno);
        exit(-1);
    }
}

int Socket::accept(InetAddress *peerAddr) {
    sockaddr_in addr;
    socklen_t len = sizeof(addr);
    bzero(&addr, len);

    int connFd = accept4(m_sockFd, (sockaddr *) &addr, &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    LOG_TRACE("connect fd: {}", connFd);
    if (connFd >= 0) {
        peerAddr->setSockAddr(addr);
    } else {
        LOG_ERROR("accept4 failed,connFd: {}, errno: {}", connFd, errno);
    }

    return connFd;
}

void Socket::shutdownWrite() {
    int shutdownRet = shutdown(m_sockFd, SHUT_WR);
    if (shutdownRet < 0) {
        LOG_ERROR("shutdown write error,shutdown ret: {}, errno: {}", shutdownRet, errno);
    }
}

void Socket::tcpNoDelay(bool delay) {
    int optVal = delay ? 1 : 0;
    setsockopt(m_sockFd, IPPROTO_TP, TCP_NODELAY, &optVal, sizeof(optVal));
}

void Socket::reuseAddr(bool reuse) {
    int optVal = reuse ? 1 : 0;
    setsockopt(m_sockFd, IPPROTO_TCP, SO_REUSEADDR, &optVal, sizeof(optVal));
}

void Socket::reusePort(bool reuse) {
    int optVal = reuse ? 1 : 0;
    setsockopt(m_sockFd, IPPROTO_TCP, SO_REUSEPORT, &optVal, sizeof(optVal));
}

void Socket::keepAlive(bool keepAlive) {
    int optVal = keepAlive ? 1 : 0;
    setsockopt(m_sockFd, SOL_SOCKET, SO_KEEPALIVE, &optVal, sizeof(optVal));
}

void Socket::closeFd() {
    if (m_sockFd > 0) {
        if (close(m_sockFd) < 0) {
            LOG_ERROR("close fd error, fd: {}, errno: {}", m_sockFd, errno);
        }
    }
}
