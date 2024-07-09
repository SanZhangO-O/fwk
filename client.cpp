#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>

#include "out/base.pb.h"

template <int I, typename... Args>
typename std::enable_if_t<I == std::tuple_size_v<std::tuple<Args...>>>
setupParameter(base::Request &request, std::tuple<Args...> tuple)
{
}

template <int I, typename... Args>
typename std::enable_if_t<I != std::tuple_size_v<std::tuple<Args...>>>
setupParameter(base::Request &request, std::tuple<Args...> tuple)
{
    request.add_parameter(std::get<I>(tuple));
    setupParameter<I + 1>(request, tuple);
}

template <typename... Args>
base::Request rpcCall(std::string name, Args... args)
{
    base::Request request;
    request.set_name(name);
    auto tuple = std::make_tuple(args...);
    setupParameter<0, Args...>(request, tuple);
    return request;
}

void wrapperSend(int fd, const std::string &data)
{
    uint32_t length = data.size();
    std::cout << "length " << length << std::endl;
    send(fd, (char *)(&length), sizeof(uint32_t), 0);
    send(fd, data.data(), data.size(), 0);
}

int main()
{
    // 创建 TCP 套接字
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    // 设置服务器地址和端口
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    if (inet_pton(AF_INET, "127.0.0.1", &(serverAddress.sin_addr)) <= 0)
    {
        std::cerr << "Invalid address" << std::endl;
        return -1;
    }

    // 连接到服务器
    if (connect(clientSocket, reinterpret_cast<struct sockaddr *>(&serverAddress), sizeof(serverAddress)) < 0)
    {
        std::cerr << "Connection failed" << std::endl;
        return -1;
    }

    // 发送数据
    std::string msg;
    base::Request request = rpcCall("AAA", std::string("ABC"), std::string("BCD"));
    // request.set_name("AAA");
    // request.add_parameter("P1");
    // request.add_parameter("P2");
    request.SerializeToString(&msg);
    wrapperSend(clientSocket, msg);

    // 关闭套接字
    close(clientSocket);

    return 0;
}
