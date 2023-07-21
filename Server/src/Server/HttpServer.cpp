/*************************************************************************
> File Name       : HttpServer.cpp
> Brief           : Http服务器
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月21日  15时53分38秒
************************************************************************/
#include "HttpServer.h"
#include "Common/http/HttpSession.h"
#include "Common/include/Log.hpp"

void HttpServer::DoAccept()
{
    _accepter.async_accept(
        [this](const std::error_code &errcode, asio::ip::tcp::socket socket)
        {
            if (errcode)
            {
                Log::error("接受连接失败：{}", errcode.message());
                return;
            }

            auto pSession = std::make_shared<Http::HttpSession>(std::move(socket));
            pSession->StartSession();

            DoAccept();
        });
}