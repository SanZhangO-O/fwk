#include <iostream>
#include <unistd.h>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <atomic>
#include <thread>

#include "O_O.hpp"
#include "defs.hpp"

int main()
{
    std::atomic_bool flag = false;
    O_O::RpcClient client("127.0.0.1", 8080);
    assert(client.start());
    {
        client.rpcCall<int>("AAA", std::string("ABC"), int(123));

        // client.rpcCallAsync<int>("AAA", [&flag](int& i){
        //     flag = true;
        // }, std::string("ABC"), int(123));
    }

    client.stop();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    // while(!flag)
    // {
    //     std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // }

    // {
    //     Struct2 struct2;
    //     struct2.struct1.i = 123;
    //     struct2.struct1.s = "ABC";
    //     struct2.ss = "DEF";

    //     bool rt = client.rpcCall<bool>("BBB", struct2);
    //     std::cout<<"B rt "<<rt<<std::endl;
    // }
    return 0;
}
