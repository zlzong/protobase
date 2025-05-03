#pragma once

#include "InetAddress.h"
#include "Callbacks.h"
#include "Channel.h"
#include "base/Timestamp.h"
#include "base/Buffer.h"

#include <memory>
#include <string>
#include <atomic>
#include <codecs/Decoder.h>

class EventLoop;

class Socket;

class Connection : public Channel {
public:
    Connection(EventLoop *eventLoop, const std::string &name, int sockFd, const InetAddress &localAddr, const InetAddress &peerAddr);

    ~Connection() override;

    const std::string &name() const;

    EventLoop *getLoop() const;

    const InetAddress &localAddress() const;

    const InetAddress &peerAddress() const;

    bool connected() const;

    void send(const std::string &buf);

    void send(const void* message, int length);

    void send(BufferPtr message);

    void shutdown();

    void forceClose();

    void setConnectionCallback(const ConnectionCallback &cb);

    void setMessageCallback(const MessageCallback &cb);

    void setWriteCompleteCallback(const WriteCompleteCallback &cb);

    void setHighWaterCallback(const HighWaterMarkCallback &cb);

    void setCloseCallback(const CloseCallback &cb);

    void onConnectionDestroy();

    void onConnectionEstablish();

private:
    void onRead(Timestamp receiveTime) override;

    void onWrite() override;

    void onClose() override;

    void onError(const std::string &errMsg) override;

    void setState(ConnectionState state);

    void forceCloseInLoop();

    void sendInLoop(const void *message, size_t len);

    void sendBufferInLoop(BufferPtr message);

    void shutdownInLoop();

private:
    std::string m_name;
    EventLoop *m_eventLoop;
    std::atomic<int> m_state;
    bool m_reading;
    std::unique_ptr<Socket> m_socket;
    const InetAddress m_localAddr;
    const InetAddress m_peerAddr;
    ConnectionCallback m_connectionCallback;
    MessageCallback m_messageCallback;
    WriteCompleteCallback m_writeCompleteCallback;
    HighWaterMarkCallback m_highWaterMarkCallback;
    CloseCallback m_closeCallback;
    size_t m_highWaterMark;
    Buffer m_inputBuffer;
    Buffer m_outputBuffer;
};
