﻿/*************************************************************************
> File Name       : server.cpp
> Brief           : 服务器
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月06日  14时41分54秒
************************************************************************/
#include "Common/include/platform.h"
#include "server.h"
#include "Common/include/log.hpp"
#include "session.h"

namespace net
{

    static thread_local bool gNetThreadFlag = false; // 网络线程标志

    Server::Server(asio::io_context &ioContext, uint16_t port)
        : _ioContext(ioContext), _accepter(ioContext,
                                           asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
    {
    }

    void Server::Start()
    {
        DoAccept();

        _netThread = std::thread(
            [this]()
            {
                gNetThreadFlag = true;
                while (true)
                {
                    try
                    {
                        _ioContext.run();
                        break; // 正常退出
                    }
                    catch (const std::exception &e)
                    {
                        Log::Error("服务器异常退出:{}", e.what());
                    }
                }
            });
    }

    // void Server::DoAccept()
    // {
    //     _accepter.async_accept(
    //         [this](const std::error_code &errcode, asio::ip::tcp::socket socket)
    //         {
    //             if (errcode)
    //             {
    //                 Log::error("接受连接失败：{}", errcode.message());
    //                 return;
    //             }

    //             auto pSession = std::make_shared<Session>(std::move(socket));
    //             pSession->StartSession();

    //             DoAccept();
    //         });
    // }
} // namespace net