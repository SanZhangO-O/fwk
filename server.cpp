#include <iostream>
#include <functional>
#include <unistd.h>

#include "out/base.pb.h"
#include "O_O.hpp"

int main()
{
    O_O::RpcMessageHandler handler;
    std::function<int(std::string, int, std::vector<int>, std::vector<std::string>, bool)> callbackAAA = [](std::string a, int b, std::vector<int> c, std::vector<std::string> d, bool e)
    {
        std::cout << "First: " << a << " Second: " << b << std::endl;
        std::cout << "Third: " << std::endl;
        for (auto i : c)
        {
            std::cout << i << " " << std::endl;
        }
        for (auto i : d)
        {
            std::cout << i << " " << std::endl;
        }
        std::cout << e << std::endl;
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
