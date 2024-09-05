#pragma once

#include <netinet/in.h>
#include <string>

class InetAddress {
public:
    explicit InetAddress(uint16_t port = 0, const std::string &ip = "127.0.0.1");

    explicit InetAddress(const sockaddr_in &addr);

    std::string getIp() const;

    std::string getIpPort() const;

    uint16_t getPort() const;

    const sockaddr_in *getSockAddrIn() const;

    sockaddr *getSockAddr();

    void setSockAddr(const sockaddr_in &addr);

private:
    sockaddr_in m_addr{};
};
