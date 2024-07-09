#include <iostream>
#include <functional>
#include <unordered_map>
#include <map>
#include <type_traits>

#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/util/delimited_message_util.h>
#include "out/base.pb.h"

namespace
{
    constexpr int MAX_EVENTS = 10;
    constexpr int BUFFER_SIZE = 4096;
}

template <int I, typename... Args>
typename std::enable_if_t<I == std::tuple_size_v<std::tuple<Args...>>>
setupTuple(std::tuple<Args...> &tuple, base::Request request)
{
}

template <int I = 0, typename... Args>
typename std::enable_if_t<I != std::tuple_size_v<std::tuple<Args...>>>
setupTuple(std::tuple<Args...> &tuple, base::Request request)
{
    std::get<I>(tuple) = request.parameter(I);
    setupTuple<I + 1, Args...>(tuple, request);
}

namespace fwk
{
    class TcpServer
    {
    public:
        TcpServer(uint32_t port, std::function<void(const std::string &data, uint32_t)> func)
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
                    m_socketFdAddressMap[clientSocket] = clientAddress;

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
                        m_socketFdAddressMap.erase(m_events[i].data.fd);
                    }
                    else
                    {
                        std::string newString;
                        newString.resize(bytesRead);
                        std::copy_n(std::begin(buffer), bytesRead, newString.begin());
                        m_procFunc(newString, m_socketFdAddressMap[m_events[i].data.fd].sin_addr.s_addr);
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
        std::function<void(const std::string &data, uint32_t)> m_procFunc;
        std::map<uint32_t, sockaddr_in> m_socketFdAddressMap;
    };
}

using namespace fwk;

class TcpMessageHandler
{
public:
    void handleMessage(const std::string &message, const uint32_t address)
    {
        std::cout << "TcpMessageHandler::handleMessage " << address << " " << message.size() << std::endl;
        // std::cout << "Received Message: " << message << std::endl;
        auto &info = m_info[address];
        info.buffer += message;
        while (true)
        {
            if (info.state == StateE::WAITING_FOR_LENGTH)
            {
                if (info.buffer.size() >= sizeof(uint32_t))
                {
                    info.length = (*(uint32_t *)(info.buffer.data()));
                    info.state = StateE::WAITING_FOR_DATA;
                    info.buffer = info.buffer.substr(4);
                    std::cout << "Get length success: " << info.length << std::endl;
                }
                else
                {
                    info.buffer = info.buffer;
                    break;
                }
            }
            else if (info.state == StateE::WAITING_FOR_DATA)
            {
                if (info.buffer.size() >= info.length)
                {
                    info.state = StateE::WAITING_FOR_LENGTH;
                    handleMessage1(info.buffer.substr(0, info.length), address);
                    std::cout << "Received Message O_O: " << info.buffer.substr(0, info.length) << std::endl;
                    info.buffer = info.buffer.substr(info.length);
                    info.length = 0;
                }
                else
                {
                    info.buffer = info.buffer;
                    break;
                }
            }
        }
    }

    void handleDisconnect(const uint32_t address)
    {
        m_info.erase(address);
    }

    void handleMessage1(std::string message, const uint32_t address)
    {
        std::cout << "handleMessage1 " << message.size() << std::endl;
        base::Request request;
        request.ParseFromString(message);
        std::cout << request.name() << std::endl;
        for (auto i = 0; i < request.parameter_size(); i++)
        {
            std::cout << request.parameter(i) << std::endl;
        }

        auto callback = m_callbackMap[request.name()];
        callback(request);
    }

    template <typename... Args>
    void registerCallback(const std::string &name, std::function<void(Args...)> callback)
    {
        auto newCallback = [callback](base::Request request)
        {
            std::tuple<Args...> parameterTuple;
            setupTuple<0, Args...>(parameterTuple, request);
            std::apply(callback, parameterTuple);
        };
        m_callbackMap[name] = newCallback;
    }

private:
    enum class StateE
    {
        WAITING_FOR_LENGTH,
        WAITING_FOR_DATA
    };
    struct Info
    {
        std::string buffer;
        StateE state = StateE::WAITING_FOR_LENGTH;
        uint32_t length = 0;
    };
    std::map<uint32_t, Info> m_info;
    std::map<std::string, std::function<void(base::Request)>> m_callbackMap;
};

int main()
{
    TcpMessageHandler handler;
    std::function<void(std::string, std::string)> callback = [](std::string a, std::string b)
    {
        std::cout << "First: " << a << " Second: " << b << std::endl;
    };
    handler.registerCallback("AAA", callback);
    TcpServer server(8080, std::bind(&TcpMessageHandler::handleMessage, &handler, std::placeholders::_1, std::placeholders::_2));
    server.run();

    return 0;
}
