#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>

int main() {
    // 创建 TCP 套接字
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    // 设置服务器地址和端口
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    if (inet_pton(AF_INET, "127.0.0.1", &(serverAddress.sin_addr)) <= 0) {
        std::cerr << "Invalid address" << std::endl;
        return -1;
    }

    // 连接到服务器
    if (connect(clientSocket, reinterpret_cast<struct sockaddr *>(&serverAddress), sizeof(serverAddress)) < 0) {
        std::cerr << "Connection failed" << std::endl;
        return -1;
    }

    // 发送数据
    const char *message = "Hello, Server!";
    if (send(clientSocket, message, strlen(message), 0) < 0) {
        std::cerr << "Send failed" << std::endl;
        return -1;
    }

    // 关闭套接字
    close(clientSocket);

    return 0;
}
