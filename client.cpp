#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "out/base.pb.h"

template<typename T>
void setupParameter_(base::Request &request,T value)
{
    assert(false&&"1");
}

template<>
void setupParameter_<int32_t>(base::Request &request, int32_t value)
{
    auto parameter = request.add_parameter();
    auto basic_value = parameter->mutable_basic_value();
    basic_value->set_int_value(value);
}

template<>
void setupParameter_<std::string>(base::Request &request, std::string value)
{
    auto parameter = request.add_parameter();
    auto basic_value = parameter->mutable_basic_value();
    basic_value->set_string_value(value);
}

template <int I = 0, typename... Args>
typename std::enable_if_t<I == std::tuple_size_v<std::tuple<Args...>>>
setupParameter(base::Request &request, std::tuple<Args...> tuple)
{
}

template <int I = 0, typename... Args>
typename std::enable_if_t<I != std::tuple_size_v<std::tuple<Args...>>>
setupParameter(base::Request &request, std::tuple<Args...> tuple)
{
    setupParameter_(request, std::get<I>(tuple));

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

void wrapSend(int fd, const std::string &data)
{
    uint32_t length = data.size();
    std::cout << "length " << length << std::endl;
    send(fd, (char *)(&length), sizeof(uint32_t), 0);
    send(fd, data.data(), data.size(), 0);
}

int main()
{
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    if (inet_pton(AF_INET, "127.0.0.1", &(serverAddress.sin_addr)) <= 0)
    {
        return -1;
    }

    if (connect(clientSocket, reinterpret_cast<struct sockaddr *>(&serverAddress), sizeof(serverAddress)) < 0)
    {
        return -1;
    }

    base::Request request = rpcCall("AAA", std::string("ABC"), int(123));
    std::string msg;
    request.SerializeToString(&msg);
    wrapSend(clientSocket, msg);

    close(clientSocket);

    return 0;
}
