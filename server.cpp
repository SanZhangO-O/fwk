#include <iostream>
#include <functional>
#include <unistd.h>
#include <vector>

#include "O_O.hpp"

int main()
{
    O_O::RpcMessageHandler handler;
    std::function<int(std::string, int)> callbackAAA = [](std::string a, int b)
    {
        std::cout << "First: " << a << " Second: " << b << std::endl;
        return 123;
    };
    handler.registerCallback("AAA", callbackAAA);
    std::function<std::vector<std::string>(void)> callbackBBB = []()
    {
        std::cout << "BBB Called" << std::endl;
        return std::vector<std::string>{"AA","BB"};
    };
    handler.registerCallback("BBB", callbackBBB);

    O_O::TcpServer server(8080, std::bind(&O_O::RpcMessageHandler::handleSocketData, &handler, std::placeholders::_1, std::placeholders::_2), std::bind(&O_O::RpcMessageHandler::handleDisconnect, &handler, std::placeholders::_1));
    server.run();

    return 0;
}
