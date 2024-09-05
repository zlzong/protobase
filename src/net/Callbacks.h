#pragma once

#include <memory>
#include <functional>

class Buffer;
class Connection;
class Timestamp;
class Connector;

using ConnectionPtr = std::shared_ptr<Connection>;
using ConnectorPtr = std::shared_ptr<Connector>;
using ConnectionCallback = std::function<void(const ConnectionPtr &)>;
using CloseCallback = std::function<void(const ConnectionPtr &)>;
using WriteCompleteCallback = std::function<void(const ConnectionPtr &)>;
using MessageCallback = std::function<void(const ConnectionPtr &, Buffer *, Timestamp)>;
using HighWaterMarkCallback = std::function<void(const ConnectionPtr &, size_t)>;
using TimerCallback = std::function<void()>;

enum ConnectionState {
    kConnecting,
    kConnected,
    kDisconnecting,
    kDisconnected
};

enum EpollState {
    kNew = -1,
    kAdded = 1,
    kDeleted = 2
};

enum PortReuseOption {
    kReusePort,
    kNonReusePort
};