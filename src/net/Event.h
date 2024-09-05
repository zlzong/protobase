#pragma once

#include "Channel.h"

#include <memory>

class EventLoop;

class Event : public std::enable_shared_from_this<Event> {
public:
    explicit Event(EventLoop *eventLoop);

    Event(EventLoop *eventLoop, int fd);

    virtual ~Event();

    virtual void onRead();

    virtual void onWrite();

    virtual void onClose();

    virtual void onError(const std::string &errMsg);

    bool enableReading();

    bool disableReading();

    bool enableWriting();

    bool disableWriting();

    int getFd() const;

    void close();

protected:
    std::unique_ptr<Channel> m_channel;
    int m_fd{-1};
    unsigned int m_event{0};
};

