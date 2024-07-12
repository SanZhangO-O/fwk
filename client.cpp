#include <iostream>
#include <unistd.h>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "O_O.hpp"
#include "out/base.pb.h"

int main()
{
    using namespace O_O;

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

    base::Request request = rpcCall("AAA", std::string("ABC"), int(123), std::vector<int>{1, 2, 3}, std::vector<std::string>{"DDD", "EEE"}, true);
    std::string msg;
    request.SerializeToString(&msg);
    O_O::sendWithLength(clientSocket, msg);

    close(clientSocket);

    return 0;
}
