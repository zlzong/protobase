#include "InetAddress.h"

#include <cstring>
#include <arpa/inet.h>

InetAddress::InetAddress(uint16_t port, const std::string &ip) {
    bzero(&m_addr, sizeof(m_addr));
    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons(port);
    m_addr.sin_addr.s_addr = inet_addr(ip.c_str());
}

InetAddress::InetAddress(const sockaddr_in &addr) : m_addr(addr) {}

std::string InetAddress::getIp() const {
    char buf[64] = {0};
    inet_ntop(AF_INET, &m_addr.sin_addr, buf, sizeof(buf));
    return buf;
}

std::string InetAddress::getIpPort() const {
    char buf[64] = {0};
    inet_ntop(AF_INET, &m_addr.sin_addr, buf, sizeof(buf));
    size_t end = strlen(buf);
    uint16_t port = ntohs(m_addr.sin_port);
    sprintf(buf + end, ":%u", port);
    return buf;
}

uint16_t InetAddress::getPort() const {
    uint16_t port = ntohs(m_addr.sin_port);
    return port;
}

const sockaddr_in *InetAddress::getSockAddrIn() const {
    return &m_addr;
}

sockaddr *InetAddress::getSockAddr() {
    return (sockaddr *) &m_addr;
}

void InetAddress::setSockAddr(const sockaddr_in &addr) {
    m_addr = addr;
}
