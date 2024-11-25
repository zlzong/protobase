#include "net/InetAddress.h"
#include "net/TcpServer.h"

void connectionEvent(const ConnectionPtr &tcpConnectionPtr) {
    bool connected = tcpConnectionPtr->connected();
    LOG_INFO("connection status: {}", connected);
}

void onMessage(const ConnectionPtr &connection, Buffer *message, Timestamp timestamp) {
    LOG_ERROR("time: {}, receive: {}", timestamp.localeString(), message->readAllAsString());
}


int main() {
    std::string mainLoopName = "main-loop";
    EventLoop eventLoop(mainLoopName);

    InetAddress listenAddr(9999, "0.0.0.0");
    TcpServer tcpServer(&eventLoop, listenAddr, "test");
    tcpServer.setThreadNum(3);
    tcpServer.setMessageCallback(onMessage);
    tcpServer.setConnectionCallback(connectionEvent);
    tcpServer.start();
    eventLoop.loop();
    return 0;
}