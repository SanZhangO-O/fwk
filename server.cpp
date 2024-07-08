#include <iostream>
#include <functional>
#include <unordered_map>

#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>

namespace
{
    constexpr int MAX_EVENTS = 10;
    constexpr int BUFFER_SIZE = 4096;
}

namespace fwk
{
    class TcpServer
    {
    public:
        TcpServer(uint32_t port, std::function<void(const std::string &data)> func)
        {
            m_procFunc = func;

            m_serverSocket = socket(AF_INET, SOCK_STREAM, 0);

            int reuse = 1;
            setsockopt(m_serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

            sockaddr_in serverAddress{};
            serverAddress.sin_family = AF_INET;
            serverAddress.sin_addr.s_addr = INADDR_ANY;
            serverAddress.sin_port = htons(port);
            bind(m_serverSocket, reinterpret_cast<struct sockaddr *>(&serverAddress), sizeof(serverAddress));

            listen(m_serverSocket, SOMAXCONN);

            epollFd = epoll_create1(0);

            epoll_event event{};
            event.events = EPOLLIN;
            event.data.fd = m_serverSocket;
            epoll_ctl(epollFd, EPOLL_CTL_ADD, m_serverSocket, &event);
        }

        ~TcpServer()
        {
            close(m_serverSocket);
            close(epollFd);
        }

        void wait()
        {
            m_numEvents = epoll_wait(epollFd, m_events, MAX_EVENTS, -1);
        }

        void step()
        {
            for (int i = 0; i < m_numEvents; i++)
            {
                if (m_events[i].data.fd == m_serverSocket)
                {
                    sockaddr_in clientAddress{};
                    socklen_t clientAddressLength = sizeof(clientAddress);
                    int clientSocket = accept(m_serverSocket, reinterpret_cast<struct sockaddr *>(&clientAddress),
                                              &clientAddressLength);

                    epoll_event event{};
                    event.events = EPOLLIN;
                    event.data.fd = clientSocket;
                    epoll_ctl(epollFd, EPOLL_CTL_ADD, clientSocket, &event);

                    std::cout << "New client connected" << std::endl;
                }
                else
                {
                    int bytesRead = read(m_events[i].data.fd, buffer, sizeof(buffer) - 1);
                    if (bytesRead <= 0)
                    {
                        epoll_ctl(epollFd, EPOLL_CTL_DEL, m_events[i].data.fd, nullptr);
                        close(m_events[i].data.fd);
                        std::cout << "Client disconnected" << std::endl;
                    }
                    else
                    {
                        buffer[bytesRead] = '\0';
                        m_procFunc(std::string(buffer));
                    }
                }
            }
        }

        void run()
        {
            while (true)
            {
                wait();
                step();
            }
        }

    private:
        char buffer[BUFFER_SIZE];
        epoll_event m_events[MAX_EVENTS];
        int m_serverSocket = 0;
        int epollFd = 0;
        int m_numEvents = 0;
        std::function<void(const std::string &data)> m_procFunc;
    };
}

using namespace fwk;

class TcpMessageHandler
{
public:
    void handleMessage(const std::string &message)
    {
        std::cout << "Received Message: " << message << std::endl;
    }
};

int main()
{
    TcpMessageHandler handler;
    TcpServer server(8080, std::bind(&TcpMessageHandler::handleMessage, &handler, std::placeholders::_1));
    server.run();

    return 0;
}
