#include "Connection.h"
#include "Socket.h"
#include "Channel.h"
#include "base/Logger.h"
#include "EventLoop.h"

Connection::Connection(EventLoop *eventLoop, const std::string &name, int sockFd, const InetAddress &localAddr, const InetAddress &peerAddr)
        : Channel(eventLoop, sockFd)
        ,m_name(name)
        ,m_eventLoop(eventLoop)
        ,m_state(kConnecting)
        ,m_reading(true)
        ,m_socket(new Socket(sockFd))
        ,m_localAddr(localAddr)
        ,m_peerAddr(peerAddr)
        ,m_highWaterMark(64 * 1024 * 1024) {
    if (!m_eventLoop) {
        LOG_ERROR("eventLoop can't be null");
        exit(-1);
    }

    setReadCallback(std::bind(&Connection::onRead, this, std::placeholders::_1));
    setWriteCallback(std::bind(&Connection::onWrite, this));
    setCloseCallback(std::bind(&Connection::onClose, this));
    setErrorCallback(std::bind(&Connection::onError, this, std::placeholders::_1));

    m_socket->keepAlive(true);
}

Connection::~Connection() = default;

const std::string &Connection::name() const {
    return m_name;
}

EventLoop *Connection::getLoop() const {
    return m_eventLoop;
}

const InetAddress &Connection::localAddress() const {
    return m_localAddr;
}

const InetAddress &Connection::peerAddress() const {
    return m_peerAddr;
}

bool Connection::connected() const {
    return m_state == kConnected;
}

void Connection::send(const std::string &buf) {
    if (m_state == kConnected) {
        if (m_eventLoop->inLoopThread()) {
            sendInLoop(buf.c_str(), buf.size());
        } else {
            m_eventLoop->runInLoop(std::bind(&Connection::sendInLoop, this, buf.c_str(), buf.size()));
        }
    }
}

void Connection::send(const void *message, int length) {
    if (m_state == kConnected) {
        if (m_eventLoop->inLoopThread()) {
            sendInLoop(message, length);
        } else {
            m_eventLoop->runInLoop(std::bind(&Connection::sendInLoop, this, message, length));
        }
    }
}

void Connection::send(BufferPtr message) {
    if (m_state == kConnected) {
        if (m_eventLoop->inLoopThread()) {
            sendBufferInLoop(message);
        } else {
            m_eventLoop->runInLoop(std::bind(&Connection::sendBufferInLoop, this, message));
        }
    }
}

void Connection::shutdown() {
    if (m_state == kConnected) {
        setState(kDisconnecting);
        m_eventLoop->runInLoop(std::bind(&Connection::shutdownInLoop, this));
    }
}

void Connection::forceClose() {
    if (m_state == kConnected || m_state == kDisconnecting) {
        setState(kDisconnecting);
        m_eventLoop->queueInLoop(std::bind(&Connection::forceCloseInLoop, this));
    }
}

void Connection::setConnectionCallback(const ConnectionCallback &cb) {
    m_connectionCallback = cb;
}

void Connection::setMessageCallback(const MessageCallback &cb) {
    m_messageCallback = cb;
}

void Connection::setWriteCompleteCallback(const WriteCompleteCallback &cb) {
    m_writeCompleteCallback = cb;
}

void Connection::setHighWaterCallback(const HighWaterMarkCallback &cb) {
    m_highWaterMarkCallback = cb;
}

void Connection::setCloseCallback(const CloseCallback &cb) {
    m_closeCallback = cb;
}

void Connection::onConnectionDestroy() {
    if (m_state == kConnected) {
        disableAll();
//        m_connectionCallback(shared_from_this());
        setState(kDisconnected);
    }

    remove();
}

void Connection::onConnectionEstablish() {
    setState(kConnected);
    tie(shared_from_this());
    enableReading();

    if (m_connectionCallback) {
        m_connectionCallback(std::static_pointer_cast<Connection>(shared_from_this()));
    } else {
        LOG_WARN("connectionCallback is nullptr");
        // exit(-1);
    }
}

void Connection::setState(ConnectionState state) {
    m_state = state;
}

void Connection::onRead(Timestamp receiveTime) {
    ssize_t nRead = m_inputBuffer.readFd(fd());
    if (nRead > 0) {
        if (m_messageCallback) {
            m_messageCallback(std::static_pointer_cast<Connection>(shared_from_this()), &m_inputBuffer, receiveTime);
        }
    } else if (nRead == 0) {
        LOG_WARN("connection reset by peer");
        onClose();
    } else {
        LOG_ERROR("read buffer from fd err:{}", std::strerror(errno));
        onError("read buffer from fd");
    }
}

void Connection::onWrite() {
    if (isWriting()) {
        size_t nWrite = m_outputBuffer.writeFd(fd());
        if (nWrite > 0) {
            m_outputBuffer.skip(nWrite);

            if (m_outputBuffer.readableBytes() == 0) {
                disableWriting();

                if (m_writeCompleteCallback) {
                    m_eventLoop->queueInLoop(std::bind(m_writeCompleteCallback, std::static_pointer_cast<Connection>(shared_from_this())));
                }

                if (m_state == kDisconnecting) {
                    shutdownInLoop();
                }
            }
        } else {
            LOG_ERROR("write to fd error,nWrite size is :{}", nWrite);
        }
    } else {
        LOG_ERROR("fd :{} is down ,no more writing ", fd());
    }
}

void Connection::onClose() {
    disableAll();
    ConnectionPtr connectionPtr(std::static_pointer_cast<Connection>(shared_from_this()));
    setState(kDisconnected);

    if (m_closeCallback) {
        m_closeCallback(connectionPtr);
    }
}

void Connection::forceCloseInLoop() {
    if (m_state == kConnected || m_state == kDisconnecting) {
        onClose();
    }
}

void Connection::onError(const std::string &errMsg) {
    disableAll();
    ConnectionPtr connectionPtr(std::static_pointer_cast<Connection>(shared_from_this()));
//    m_connectionCallback(connectionPtr);

    // todo 评估要不要添加error call back
    if (m_closeCallback) {
        m_closeCallback(connectionPtr);
    }

    setState(kDisconnected);
}

void Connection::sendInLoop(const void *message, size_t len) {
    ssize_t nWrote = 0;
    size_t remain = len;
    bool faultError = false;

    if (m_state == kDisconnected) {
        LOG_ERROR("connection disconnected,writing cancelled");
        return;
    }

    if (!isWriting() && m_inputBuffer.readableBytes() == 0) {
        nWrote = write(fd(), message, len);
        if (nWrote > 0) {
            remain = len - nWrote;
            if (remain == 0 && m_writeCompleteCallback) {
                m_eventLoop->queueInLoop(std::bind(m_writeCompleteCallback, std::static_pointer_cast<Connection>(shared_from_this())));
            }
        } else {
            nWrote = 0;
            if (errno != EWOULDBLOCK) {
                if (errno == EPIPE || errno == ECONNRESET) {
                    faultError = true;
                }
            }
        }
    }

    if (!faultError && remain > 0) {
        size_t oldLen = m_outputBuffer.readableBytes();

        if (oldLen + remain > m_highWaterMark && oldLen < m_highWaterMark && m_highWaterMarkCallback) {
            m_eventLoop->queueInLoop(std::bind(m_highWaterMarkCallback, std::static_pointer_cast<Connection>(shared_from_this()), oldLen + remain));
        }

        m_outputBuffer.append((char *) message + nWrote, remain);
        if (!isWriting()) {
            enableWriting();
        }
    }
}

void Connection::sendBufferInLoop(BufferPtr message) {
    size_t len = message->readableBytes();
    ssize_t nWrote = 0;
    size_t remain = len;
    bool faultError = false;

    if (m_state == kDisconnected) {
        LOG_ERROR("connection disconnected,writing cancelled");
        return;
    }

    if (!isWriting() && m_inputBuffer.readableBytes() == 0) {
        nWrote = write(fd(), message->peek(), len);
        if (nWrote > 0) {
            remain = len - nWrote;
            if (remain == 0 && m_writeCompleteCallback) {
                m_eventLoop->queueInLoop(std::bind(m_writeCompleteCallback, std::static_pointer_cast<Connection>(shared_from_this())));
            }
        } else {
            nWrote = 0;
            if (errno != EWOULDBLOCK) {
                if (errno == EPIPE || errno == ECONNRESET) {
                    faultError = true;
                }
            }
        }
    }

    if (!faultError && remain > 0) {
        size_t oldLen = m_outputBuffer.readableBytes();

        if (oldLen + remain > m_highWaterMark && oldLen < m_highWaterMark && m_highWaterMarkCallback) {
            m_eventLoop->queueInLoop(std::bind(m_highWaterMarkCallback, std::static_pointer_cast<Connection>(shared_from_this()), oldLen + remain));
        }

        m_outputBuffer.append(message->peek() + nWrote, remain);
        if (!isWriting()) {
            enableWriting();
        }
    }
}

void Connection::shutdownInLoop() {
    if (isWriting()) {
        m_socket->shutdownWrite();
    }
}
