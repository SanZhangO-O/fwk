#include <iostream>
#include <unistd.h>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "O_O.hpp"
#include "out/base.pb.h"

int main()
{
    O_O::RpcClient client("127.0.0.1", 8080);
    client.start();
    {
        auto rt = client.rpcCall<int>("AAA", std::string("ABC"), int(123), std::vector<int>{1, 2, 3}, std::vector<std::string>{"DDD", "EEE"}, true);
        std::cout << rt << std::endl;
    }
    {
        auto rt = client.rpcCall<std::vector<std::string>>("BBB");
        for(auto i:rt)
        {
            std::cout<<i<<std::endl;
        }
    }
    return 0;
}
