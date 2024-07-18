#include <iostream>
#include <unistd.h>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "O_O.hpp"
#include "defs.hpp"

int main()
{
    O_O::RpcClient client("127.0.0.1", 8080);
    client.start();
    {
        // auto rt = client.rpcCall<int>("AAA", std::string("ABC"), int(123), std::vector<int>{1, 2, 3}, std::vector<std::string>{"DDD", "EEE"}, true);
        auto rt = client.rpcCall<int>("AAA", std::string("ABC\\"), int(123));
        std::cout << rt << std::endl;
    }
    {
        Struct2 struct2;
        struct2.struct1.i = 123;
        struct2.struct1.s = "ABC";
        struct2.ss = "DEF";

        bool rt = client.rpcCall<bool>("BBB", struct2);
        std::cout<<"B rt "<<rt<<std::endl;
    }
    return 0;
}
