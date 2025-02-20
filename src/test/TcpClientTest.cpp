#include "base/Logger.h"
#include "net/EventLoopThread.h"
#include "net/InetAddress.h"
#include "net/TcpClient.h"
#include "net/EventLoop.h"


EventLoopThread eventLoopThread;
int main() {
    EventLoop *pLoop = eventLoopThread.startLoop();
    InetAddress inetAddress(9999, "127.0.0.1");
    TcpClient client(pLoop,inetAddress,"test");

    client.setMessageCallback([](const ConnectionPtr &connection, Buffer *message, Timestamp timestamp) {
        std::string receiveMessage = message->readAllAsString();
        LOG_INFO("time: {}, mesasge: {}", timestamp.localeString(), receiveMessage);
        connection->send(receiveMessage);
    });

    client.setConnectionCallback([](const ConnectionPtr & connection) {
        connection->send("123", 3);
    });

    client.enableRetry();
    client.clientConnect();

    int timerId = pLoop->runEvery(3*1000,[&]{
        client.connection()->send("hello", 5);
    });

    std::this_thread::sleep_for(std::chrono::seconds(20));

    pLoop->cancel(timerId);

    std::this_thread::sleep_for(std::chrono::seconds(100));
}