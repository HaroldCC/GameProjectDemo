/*************************************************************************
> File Name       : server.h
> Brief           : 服务器
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月06日  14时35分17秒
************************************************************************/
#pragma once
#include <thread>
#include "asio.hpp"

#include "Common/database/QueryCallback.h"

namespace net
{
    class IServer
    {
    public:
        IServer(const IServer &)            = delete;
        IServer(IServer &&)                 = delete;
        IServer &operator=(const IServer &) = delete;
        IServer &operator=(IServer &&)      = delete;
        IServer(asio::io_context &ioContext, uint16_t port);
        virtual ~IServer() = default;

        void Start();

    protected:
        virtual void Update();
        virtual void DoAccept() = 0;

    protected:
        std::thread                                  _netThread;
        asio::io_context                            &_ioContext;
        asio::ip::tcp::acceptor                      _accepter;
        asio::steady_timer                           _updateTimer;
        std::vector<std::shared_ptr<class ISession>> _sessions;
        QueryCallbackProcessor                       _processor;
    };
} // namespace net