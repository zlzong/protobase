#include "Acceptor.h"
#include "base/Logger.h"
#include "InetAddress.h"

#include <sys/socket.h>
#include <unistd.h>

static int createNonblocking() {
    int sockFd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockFd < 0) {
        LOG_ERROR("create socket failed,errno: {}", errno);
        exit(-1);
    }

    return sockFd;
}

Acceptor::Acceptor(EventLoop *eventLoop, const InetAddress &listenAddr)
        : Channel(eventLoop, createNonblocking()), m_eventLoop(eventLoop), m_socket(fd()),
          m_idleFd(open("/dev/null", O_RDONLY | O_CLOEXEC)) {
    m_socket.reuseAddr(true);
    m_socket.reusePort(true);
    m_socket.bindAddress(listenAddr);
    setReadCallback(std::bind(&Acceptor::onRead, this, std::placeholders::_1));
}

Acceptor::~Acceptor() {
    disableAll();
    remove();
}

void Acceptor::setNweConnectionCallback(const Acceptor::NweConnectionCallback &cb) {
    m_newConnectionCallback = cb;
}

bool Acceptor::listenStatus() const {
    return m_listening;
}

void Acceptor::listen() {
    m_listening = true;
    m_socket.listen();
    enableReading();
}

void Acceptor::onRead(Timestamp receiveTime) {
    InetAddress peerAddr;
    int connFd = m_socket.accept(&peerAddr);
    if (connFd >= 0) {
        LOG_TRACE("new connection established: {}, receiveTime: {}", peerAddr.getIpPort(), receiveTime.localeString());
        if (m_newConnectionCallback) {
            m_newConnectionCallback(connFd, peerAddr);
        } else {
            LOG_ERROR("no connection callback for Acceptor, connection can't be establish");
            close(connFd);
        }
    } else {
        LOG_ERROR("accept error, errno :{}", errno);
        if (errno == EMFILE) {
            LOG_ERROR("sockFd reached limit!");
            close(m_idleFd);
            m_idleFd = accept(fd(), nullptr, nullptr);
            close(m_idleFd);
            m_idleFd = open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}
