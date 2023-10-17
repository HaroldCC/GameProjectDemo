/*************************************************************************
> File Name       : HttpServer.h
> Brief           : Http服务器
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月21日  15时56分00秒
************************************************************************/
#pragma once
#include "Common/net/Server.h"
#include "Common/http/HttpSession.h"

class HttpServer final : public net::IServer
{
private:
    using net::IServer::IServer;

protected:
    void DoAccept() override;

    std::optional<Http::HttpResponse> GetUserInfo(const Http::HttpRequest &request);
    std::optional<Http::HttpResponse> Sql(const Http::HttpRequest &request);

    void InitHttpRouter(const std::shared_ptr<Http::HttpSession> &pSession);
};