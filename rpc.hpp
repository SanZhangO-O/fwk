#pragma once

#include <functional>

class RpcServer
{
public:
    void start()
    {
    }
    void stop()
    {
    }

    template <typename T, typename... Args>
    void serveProdecure(std::function<T(Args...)> f)
    {

    }
};