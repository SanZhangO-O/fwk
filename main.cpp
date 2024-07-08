#include <iostream>
#include <sys/epoll.h>
#include <unistd.h>
#include <arpa/inet.h>

constexpr int MAX_EVENTS = 10;
constexpr int BUFFER_SIZE = 1024;

int main() {
    // 创建 TCP 套接字
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    // 设置地址重用选项
    int reuse = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    // 绑定地址和端口
    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(8080);
    bind(serverSocket, reinterpret_cast<struct sockaddr *>(&serverAddress), sizeof(serverAddress));

    // 开始监听
    listen(serverSocket, SOMAXCONN);

    // 创建 epoll 实例
    int epollFd = epoll_create1(0);

    // 添加服务器套接字到 epoll 监听列表中
    epoll_event event{};
    event.events = EPOLLIN;
    event.data.fd = serverSocket;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, serverSocket, &event);

    // 创建用于存储事件的数组
    epoll_event events[MAX_EVENTS];

    while (true) {
        // 等待事件发生
        int numEvents = epoll_wait(epollFd, events, MAX_EVENTS, -1);

        // 遍历事件并处理
        for (int i = 0; i < numEvents; i++) {
            // 当有新连接建立时，接受连接并将其添加到 epoll 监听列表中
            if (events[i].data.fd == serverSocket) {
                sockaddr_in clientAddress{};
                socklen_t clientAddressLength = sizeof(clientAddress);
                int clientSocket = accept(serverSocket, reinterpret_cast<struct sockaddr *>(&clientAddress),
                                          &clientAddressLength);

                event.events = EPOLLIN;
                event.data.fd = clientSocket;
                epoll_ctl(epollFd, EPOLL_CTL_ADD, clientSocket, &event);

                std::cout << "New client connected" << std::endl;
            } 
            // 当有数据可读时，读取数据并进行处理
            else {
                char buffer[BUFFER_SIZE];
                int bytesRead = read(events[i].data.fd, buffer, sizeof(buffer) - 1);
                if (bytesRead <= 0) {
                    // 客户端关闭连接
                    epoll_ctl(epollFd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
                    close(events[i].data.fd);
                    std::cout << "Client disconnected" << std::endl;
                } else {
                    buffer[bytesRead] = '\0';
                    std::cout << "Received data: " << buffer << std::endl;

                    // 这里可以进行自定义的数据处理逻辑
                }
            }
        }
    }

    // 关闭套接字和 epoll 实例
    close(serverSocket);
    close(epollFd);

    return 0;
}
