﻿/*************************************************************************
> File Name       : server.h
> Brief           : 服务器
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月06日  14时35分17秒
************************************************************************/
#pragma once
#include "asio.hpp"

namespace net
{
    class Server
    {
    public:
        Server(const Server &)            = delete;
        Server(Server &&)                 = delete;
        Server &operator=(const Server &) = delete;
        Server &operator=(Server &&)      = delete;
        explicit Server(asio::io_context &ioContext, uint16_t port);
        virtual ~Server() = default;

        void Start()
        {
            DoAccept();
        }

    private:
        void DoAccept();

    private:
        asio::io_context       &_ioContext;
        asio::ip::tcp::acceptor _accepter;
    };
} // namespace net