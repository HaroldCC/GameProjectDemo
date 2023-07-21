﻿/*************************************************************************
> File Name       : HttpSession.h
> Brief           : http会话
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月13日  19时59分35秒
************************************************************************/

#pragma once
#include "net/session.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpRouter.h"

namespace Http
{
    class HttpSession final : public net::ISession
    {
    public:
        void AddRouter(boost::beast::http::verb method, std::string_view path, HttpHandlerFunc handler);

    protected:
        void ReadHandler() override;

    private:
        HttpRequest  _req;
        HttpResponse _rep;
        HttpRouter   _router;
    };
} // namespace Http