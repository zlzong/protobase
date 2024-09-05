#pragma once

class InetAddress;

class Socket {
public:
    explicit Socket(int sockFd);

    ~Socket() = default;

    int fd() const;

    void bindAddress(const InetAddress &localAddr) const;

    void listen();

    int accept(InetAddress *peerAddr);

    void shutdownWrite();

    void tcpNoDelay(bool delay);

    void reuseAddr(bool reuse);

    void reusePort(bool reuse);

    void keepAlive(bool keepAlive);

    void closeFd();

private:
    const int m_sockFd;
};
