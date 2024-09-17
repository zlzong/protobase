#include "Connector.h"
#include "EventLoop.h"
#include "base/Logger.h"

const int Connector::kMaxRetryIntervalMillSeconds;
const int Connector::kInitRetryIntervalMillSeconds;

Connector::Connector(EventLoop *eventLoop, const InetAddress &serverAddr, int retryInterval)
        : Channel(eventLoop)
        ,m_serverAddr(serverAddr)
        ,m_connect(false)
        ,m_state(kDisconnected)
        ,m_retryIntervalMillSeconds(retryInterval) {

}

Connector::~Connector() = default;

void Connector::start() {
    m_connect = true;
    m_retryIntervalMillSeconds = kInitRetryIntervalMillSeconds;
    ownerLoop()->runInLoop(std::bind(&Connector::startInLoop, this));
}


void Connector::restart() {
    setState(kDisconnected);
    m_connect = true;
    startInLoop();
}

void Connector::stop() {
    m_connect = false;
    ownerLoop()->runInLoop(std::bind(&Connector::stopInLoop, this));
}

void Connector::startInLoop() {
    LOG_INFO("m_connect: {}", m_connect);
    if (m_connect) {
        connect();
    }
}

void Connector::stopInLoop() {
    if (m_state == kConnecting) {
        setState(kDisconnected);
        int sockFd = removeChannel();
        retry(sockFd);
    }
}

void Connector::connect() {
    LOG_INFO("connect start with server address: {}", m_serverAddr.getIpPort().c_str());
    int sockFd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockFd == -1) {
        LOG_ERROR("create non block socket error, errno: {}", errno);
        return;
    }
    sockaddr *pSockAddr = m_serverAddr.getSockAddr();
    int connectRet = ::connect(sockFd, pSockAddr, sizeof(sockaddr));
    int errorNo = connectRet == 0 ? 0 : errno;
    LOG_INFO("connect status: {}, errno: {}", connectRet, errorNo);
    switch (errorNo) {
        case 0:
            // 操作正在进行中。对于非阻塞套接字,这表示连接请求已经发起但尚未完成
        case EINPROGRESS:
            // 系统调用被信号中断
        case EINTR:
            // 套接字已经连接
        case EISCONN:
            connecting(sockFd);
            break;
            // 资源暂时不可用,可以稍后重试
        case EAGAIN:
            // 地址已在使用中
        case EADDRINUSE:
            // 请求的地址不可用。可能是尝试绑定的IP地址在本机不存在
        case EADDRNOTAVAIL:
            // 连接被目标机器积极拒绝,通常因为目标端口没有应用程序监听
        case ECONNREFUSED:
            // 网络不可达。可能是由于路由问题或防火墙设置
        case ENETUNREACH:
            retry(sockFd);
            break;
            // 权限被拒绝。可能是因为尝试绑定特权端口(小于1024)而没有足够权限
        case EACCES:
            // 操作不被允许。类似EACCES,但更一般化的权限错误
        case EPERM:
            // 地址族不被支持
        case EAFNOSUPPORT:
            // 一个非阻塞操作已经在进行中
        case EALREADY:
            // 错误的文件描述符
        case EBADF:
            // 指针参数指向无效的内存地址
        case EFAULT:
            // 文件描述符不是一个套接字
        case ENOTSOCK:
            LOG_ERROR("connect error with errno: {}", errorNo);
            ::close(sockFd);
            setFd(-1);
            break;
        default:
            LOG_ERROR("unexpected error in connect with errno: {}", errorNo);
            ::close(sockFd);
            setFd(-1);
            break;
    }
}

void Connector::connecting(int sockFd) {
    setState(kConnecting);

    setFd(sockFd);
    setWriteCallback(std::bind(&Connector::onWrite, this));
    setErrorCallback(std::bind(&Connector::onError, this, std::placeholders::_1));
    enableWriting();
    LOG_TRACE("connector state is:{}", static_cast<int>(m_state));
}

void Connector::onWrite() {
    LOG_TRACE("connector state is:{}", static_cast<int>(m_state));
    if (m_state == kConnecting) {
        int sockFd = removeChannel();
        int err;
        socklen_t optLen = static_cast<socklen_t>(sizeof(err));
        ::getsockopt(sockFd, SOL_SOCKET, SO_ERROR, &err, &optLen);
        if (err) {
            retry(sockFd);
        } else {
            setState(kConnected);
            if (m_connect) {
                if (m_newConnectionCallback) {
                    m_newConnectionCallback(sockFd);
                } else {
                    LOG_ERROR("connection callback is nullptr");
                    ::close(sockFd);
                }
            } else {
                LOG_ERROR("m_connect value is: {}" ,m_connect);
                ::close(sockFd);
            }
        }
    } else {
        LOG_ERROR("connector state error: {}", static_cast<int>(m_state));
    }
}

void Connector::onError(const std::string &errMsg) {
    if (m_state == kConnecting) {
        int sockFd = removeChannel();
        int err;
        auto optLen = static_cast<socklen_t>(sizeof(err));
        ::getsockopt(sockFd, SOL_SOCKET, SO_ERROR, &err, &optLen);
        switch (err) {
        case ECONNREFUSED:
            LOG_ERROR("connection refused");
            break;
        case ETIMEDOUT:
            LOG_ERROR("connection timed out");
            break;
        case ECONNRESET:
            LOG_ERROR("connection reset by peer");
            break;
        case EHOSTUNREACH:
            LOG_ERROR("no route to host");
            break;
        case ENETUNREACH:
            LOG_ERROR("network is unreachable");
            break;
        case EADDRINUSE:
            LOG_ERROR("address already in use");
            break;
        default:
            LOG_ERROR("unknown socket error: {}", err);
        }

        retry(sockFd);
    }
}

void Connector::retry(int sockFd) {
    ::close(sockFd);
    setState(kDisconnected);

    if (m_connect) {
        LOG_INFO("retry connect, retry interval ms: {}, max retry interval ms: {}", m_retryIntervalMillSeconds, kMaxRetryIntervalMillSeconds);
        ownerLoop()->runAfter(m_retryIntervalMillSeconds * 1000, [this]{startInLoop();});
        m_retryIntervalMillSeconds = std::min(m_retryIntervalMillSeconds * 2, kMaxRetryIntervalMillSeconds);
    } else {
        LOG_INFO("connect may stop, m_connect: {}", m_connect);
    }
}

int Connector::removeChannel() {
    disableAll();
    remove();
    int sockFd = fd();
    ownerLoop()->queueInLoop(std::bind(&Connector::resetChannel, this));
    return sockFd;
}

void Connector::resetChannel() {
//    m_channel.reset();
//    closeFd();
}
