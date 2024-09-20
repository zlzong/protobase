add_executable(TcpClientTest TcpClientTest.cpp)
target_link_libraries(TcpClientTest base net)

add_executable(TcpServerTest TcpServerTest.cpp)
target_link_libraries(TcpServerTest base net)