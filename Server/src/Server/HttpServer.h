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

class HttpServer final : public net::Server
{
protected:
    void DoAccept() override;
};