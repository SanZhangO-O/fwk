#pragma once

#include "json.hpp"

#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#include <future>
#include <unistd.h>
#include <thread>
#include <memory>
#include <type_traits>
#include <functional>
#include <string>
#include <map>
#include <cassert>

namespace O_O
{
    class TcpMessagehandler
    {
    public:
        static void sendWithLength(const int fd, const std::string &data)
        {
            uint32_t length = data.size();
            send(fd, (char *)(&length), sizeof(uint32_t), 0);
            send(fd, data.data(), data.size(), 0);
        }

        void handleSocketData(const std::string &message, const int fd)
        {
            auto &info = m_info[fd];
            info.buffer += message;
            while (true)
            {
                if (info.state == ParsingStateE::WAITING_FOR_LENGTH)
                {
                    if (info.buffer.size() >= sizeof(uint32_t))
                    {
                        info.length = (*(uint32_t *)(info.buffer.data()));
                        info.state = ParsingStateE::WAITING_FOR_DATA;
                        info.buffer = info.buffer.substr(4);
                    }
                    else
                    {
                        info.buffer = info.buffer;
                        break;
                    }
                }
                else if (info.state == ParsingStateE::WAITING_FOR_DATA)
                {
                    if (info.buffer.size() >= info.length)
                    {
                        info.state = ParsingStateE::WAITING_FOR_LENGTH;
                        m_onMessageCallback(info.buffer.substr(0, info.length), fd);
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

        void handleDisconnect(const int fd)
        {
            m_info.erase(fd);
        }

        std::function<void(const std::string &, int)> m_onMessageCallback;

    private:
        enum class ParsingStateE
        {
            WAITING_FOR_LENGTH,
            WAITING_FOR_DATA
        };
        struct Info
        {
            std::string buffer;
            ParsingStateE state = ParsingStateE::WAITING_FOR_LENGTH;
            uint32_t length = 0;
        };
        std::map<int, Info> m_info;
    };

    class ConnectionInfo
    {
    public:
        int fd = 0;
    };

    class RpcMessageHandler
    {
    public:
        RpcMessageHandler()
        {
            m_tcpMessageHandler.m_onMessageCallback = std::bind(&RpcMessageHandler::handleMessage, this, std::placeholders::_1, std::placeholders::_2);
        }

        void handleConnect(const int fd)
        {
            std::cout << "RpcMessageHandler::handleConnect" << std::endl;
            ConnectionInfo{fd};
            auto item = std::make_shared<ConnectionInfo>();
            item->fd = fd;
            m_fdConnectionMap[fd] = item;
        }

        void handleDisconnect(const int fd)
        {
            std::cout << "RpcMessageHandler::handleDisconnect" << std::endl;
            m_fdConnectionMap.erase(fd);
            m_tcpMessageHandler.handleDisconnect(fd);
        }

        void handleSocketData(const std::string &message, const int fd)
        {
            m_tcpMessageHandler.handleSocketData(message, fd);
        }

        void handleMessage(const std::string &message, const int fd)
        {
            int index = 0;
            int procedureIndex = 0;
            std::string funcName;
            std::string parameterString;
            decode(message, index, funcName, procedureIndex, parameterString);
            auto callback = m_nameCallbackMap[funcName];
            callback(parameterString, procedureIndex, fd);
        }

        template <typename ReturnType, typename CallbackType, typename tupleType>
        std::enable_if_t<std::is_same_v<ReturnType, void>, std::string>
        applyAndGetReturnMessage(CallbackType callback, const int procedureId, const tupleType &parameterTuple)
        {
            std::apply(callback, parameterTuple);
            auto dataToSend = encode(true, procedureId, std::string());
            return dataToSend;
        }

        template <typename ReturnType, typename CallbackType, typename tupleType>
        std::enable_if_t<!std::is_same_v<ReturnType, void>, std::string>
        applyAndGetReturnMessage(CallbackType callback, const int procedureId, const tupleType &parameterTuple)
        {
            auto result = std::apply(callback, parameterTuple);
            std::string rtString;
            serializeElement(rtString, result);
            auto dataToSend = encode(true, procedureId, rtString);
            return dataToSend;
        }

        template <typename ReturnType, typename... Args>
        void registerCallback(const std::string &name, std::function<ReturnType(Args...)> callback)
        {
            auto newCallback = [this, callback](std::string message, int procedureId, int fd)
            {
                int index = 0;
                std::tuple<std::decay_t<Args>...> parameterTuple;
                deserializeElement(message, index, parameterTuple);
                auto resultData = applyAndGetReturnMessage<ReturnType, decltype(callback), decltype(parameterTuple)>(callback, procedureId, parameterTuple);
                TcpMessagehandler::sendWithLength(fd, resultData);
            };
            m_nameCallbackMap[name] = newCallback;
        }

        template <typename ReturnType, typename... Args>
        void registerCallback(const std::string &name, std::function<ReturnType(std::weak_ptr<ConnectionInfo>, Args...)> callback)
        {
            auto newCallback = [this, callback](std::string message, int procedureId, int fd)
            {
                int index = 0;
                std::tuple<std::decay_t<Args>...> parameterTuple;
                deserializeElement(message, index, parameterTuple);
                auto newTuple = std::tuple_cat(std::make_tuple<std::weak_ptr<ConnectionInfo>>(m_fdConnectionMap[fd]), parameterTuple);
                auto resultData = applyAndGetReturnMessage<ReturnType, decltype(callback), decltype(newTuple)>(callback, procedureId, newTuple);
                TcpMessagehandler::sendWithLength(fd, resultData);
            };
            m_nameCallbackMap[name] = newCallback;
        }

    private:
        std::map<std::string, std::function<void(std::string, int, int)>> m_nameCallbackMap;
        TcpMessagehandler m_tcpMessageHandler;
        std::map<int, std::shared_ptr<ConnectionInfo>> m_fdConnectionMap;
    };

    namespace
    {
        constexpr int MAX_EVENTS = 10;
        constexpr int BUFFER_SIZE = 4096;
    }

    class TcpServer
    {
    public:
        TcpServer(int port, std::function<void(const std::string &data, int)> func, std::function<void(int)> connectedFunc, std::function<void(int)> disconnectedFunc)
        {
            m_procFunc = func;
            m_connectedFunc = connectedFunc;
            m_disconnectedFunc = disconnectedFunc;

            m_serverSocket = socket(AF_INET, SOCK_STREAM, 0);

            int reuse = 1;
            setsockopt(m_serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

            sockaddr_in serverAddress{};
            serverAddress.sin_family = AF_INET;
            serverAddress.sin_addr.s_addr = INADDR_ANY;
            serverAddress.sin_port = htons(port);
            bind(m_serverSocket, reinterpret_cast<struct sockaddr *>(&serverAddress), sizeof(serverAddress));

            listen(m_serverSocket, SOMAXCONN);

            m_epollFd = epoll_create1(0);

            epoll_event event{};
            event.events = EPOLLIN;
            event.data.fd = m_serverSocket;
            epoll_ctl(m_epollFd, EPOLL_CTL_ADD, m_serverSocket, &event);
        }

        ~TcpServer()
        {
            close(m_serverSocket);
            close(m_epollFd);
        }

        void wait()
        {
            m_numEvents = epoll_wait(m_epollFd, m_events, MAX_EVENTS, -1);
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
                    epoll_ctl(m_epollFd, EPOLL_CTL_ADD, clientSocket, &event);
                    m_connectedFunc(clientSocket);
                }
                else
                {
                    int bytesRead = read(m_events[i].data.fd, buffer, sizeof(buffer) - 1);
                    if (bytesRead <= 0)
                    {
                        epoll_ctl(m_epollFd, EPOLL_CTL_DEL, m_events[i].data.fd, nullptr);
                        close(m_events[i].data.fd);
                        m_disconnectedFunc(m_events[i].data.fd);
                    }
                    else
                    {
                        std::string newString;
                        newString.resize(bytesRead);
                        std::copy_n(std::begin(buffer), bytesRead, newString.begin());
                        m_procFunc(newString, m_events[i].data.fd);
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
        int m_epollFd = 0;
        int m_numEvents = 0;
        std::function<void(const std::string &data, int)> m_procFunc;
        std::function<void(int)> m_connectedFunc;
        std::function<void(int)> m_disconnectedFunc;
    };

    // Client
    class RpcClient
    {
    public:
        RpcClient(const std::string &address, const uint16_t port) : m_address(address), m_port(port)
        {
        }

        ~RpcClient()
        {
            if (m_socket != 0)
            {
                close(m_socket);
            }
            if (m_epollFd != 0)
            {
                close(m_epollFd);
            }
        }

        bool start()
        {
            assert(m_socket == 0);
            m_socket = socket(AF_INET, SOCK_STREAM, 0);

            sockaddr_in serverAddress{};
            serverAddress.sin_family = AF_INET;
            serverAddress.sin_port = htons(m_port);
            if (inet_pton(AF_INET, m_address.data(), &(serverAddress.sin_addr)) <= 0)
            {
                return false;
            }

            if (connect(m_socket, reinterpret_cast<struct sockaddr *>(&serverAddress), sizeof(serverAddress)) < 0)
            {
                return false;
            }

            m_epollFd = epoll_create1(0);

            epoll_event event{};
            event.events = EPOLLIN;
            event.data.fd = m_socket;
            epoll_ctl(m_epollFd, EPOLL_CTL_ADD, m_socket, &event);

            m_threadRunning = true;
            m_thread = std::make_unique<std::thread>([this]()
                                                     {
                                                         TcpMessagehandler tcpMessagehandler;
                                                         tcpMessagehandler.m_onMessageCallback = [this](const std::string &data, int fd)
                                                         {
                                                             int receivedProcedureId = 0;
                                                             int index = 0;
                                                             bool isSuccess = false;
                                                             std::string returnValueString;
                                                             decode(data, index, isSuccess, receivedProcedureId, returnValueString);
                                                             auto callback = m_callbacks[receivedProcedureId];
                                                             callback(returnValueString);
                                                         };
                                                         epoll_event events[MAX_EVENTS];
                                                         char buffer[BUFFER_SIZE];

                                                         while (m_threadRunning)
                                                         {
                                                             auto numEvents = epoll_wait(m_epollFd, events, MAX_EVENTS, 100);
                                                             for (int i = 0; i < numEvents; i++)
                                                             {
                                                                 if (events[i].data.fd == m_socket)
                                                                 {
                                                                     int bytesRead = read(events[i].data.fd, buffer, sizeof(buffer) - 1);
                                                                     if (bytesRead <= 0)
                                                                     {
                                                                         epoll_ctl(m_epollFd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
                                                                         close(events[i].data.fd);
                                                                         goto exitFunc;
                                                                     }
                                                                     else
                                                                     {
                                                                         std::string newString;
                                                                         newString.resize(bytesRead);
                                                                         std::copy_n(std::begin(buffer), bytesRead, newString.begin());
                                                                         std::cout << "Recv data" << std::endl;
                                                                         tcpMessagehandler.handleSocketData(newString, m_socket);
                                                                     }
                                                                 }
                                                             }
                                                         }
                                                         exitFunc:
                                                         std::cout << "Thread exit" << std::endl;
                                                         m_thread.reset(); });
            m_thread->detach();
            return true;
        }

        void stop()
        {
            m_threadRunning = false;
            // Wait for thread finished
            close(m_socket);
            m_socket = 0;
            close(m_epollFd);
            m_epollFd = 0;
        }

        template <typename T, typename... Args>
        T rpcCall(const std::string &name, const Args &...args)
        {
            std::promise<T> promise;
            auto future = promise.get_future();
            std::function<void(T & t)> func = [&promise](T &t)
            { promise.set_value(t); };
            rpcCallAsync(name, func, args...);
            return future.get();
        }

        template <typename T, typename... Args>
        void rpcCallAsync(const std::string &name, std::function<void(T &t)> callback, const Args &...args)
        {
            int procedureId = m_procedureId++;
            auto callbackNew = [callback](const std::string &rtString)
            {
                int index = 0;
                T t;
                deserializeElement(rtString, index, t);
                callback(t);
            };
            m_callbacks[procedureId] = callbackNew;
            std::string parameterString;
            serializeElement(parameterString, std::make_tuple(args...));
            auto dataToSend = encode(name, procedureId, parameterString);
            TcpMessagehandler::sendWithLength(m_socket, dataToSend);
        }

    private:
        std::string m_address;
        uint16_t m_port = 0;
        int m_socket = 0;
        int m_epollFd = 0;
        int m_procedureId = 1;
        std::map<int, std::function<void(const std::string &)>> m_callbacks;
        std::unique_ptr<std::thread> m_thread;
        std::atomic_bool m_threadRunning = true;
    };
}
