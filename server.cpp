#include <iostream>
#include <functional>
#include <unistd.h>
#include <vector>

#include "O_O.hpp"
#include "defs.hpp"

int main()
{
    O_O::RpcMessageHandler handler;
    std::function<int(std::string, int)> callbackAAA = [](std::string a, int b)
    {
        std::cout << "BBB Called" << std::endl;
        std::cout << "First: " << a << " Second: " << b << std::endl;
        return 123;
    };
    handler.registerCallback("AAA", callbackAAA);
    std::function<bool(Struct2)> callbackBBB = [](Struct2 struct2)
    {
        std::cout << "BBB Called" << std::endl;
        std::cout << struct2.struct1.i << " " << struct2.struct1.s << " " << struct2.ss << std::endl;
    };
    handler.registerCallback("BBB", callbackBBB);

    O_O::TcpServer server(8080, std::bind(&O_O::RpcMessageHandler::handleSocketData, &handler, std::placeholders::_1, std::placeholders::_2), std::bind(&O_O::RpcMessageHandler::handleDisconnect, &handler, std::placeholders::_1));
    server.run();

    return 0;
}
