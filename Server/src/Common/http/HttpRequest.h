﻿/*************************************************************************
> File Name       : HttpRequest.h
> Brief           : http请求
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月13日  19时45分54秒
************************************************************************/
#pragma once
#include <map>
#include <string_view>

namespace Http
{
    enum class WebStatus
    {
        Ok          = 200,
        Auth        = 401,
        Error       = 404,
        InterError  = 500,
        NoImplement = 501,
        Timeout     = 503,
    };

    enum class HttpRequestType
    {
        Get     = 1 << 0,
        Post    = 1 << 1,
        Head    = 1 << 2,
        Put     = 1 << 3,
        Delete  = 1 << 4,
        Options = 1 << 5,
        Trace   = 1 << 6,
        Connect = 1 << 7,
        Patch   = 1 << 8,
    };

    class HttpRequest
    {
    public:
        HttpRequest()                               = default;
        HttpRequest(HttpRequest &&)                 = default;
        HttpRequest &operator=(HttpRequest &&)      = default;
        HttpRequest(const HttpRequest &)            = default;
        HttpRequest &operator=(const HttpRequest &) = default;
        ~HttpRequest()                              = default;

    private:
        WebStatus Parse(std::string_view content);

    private:
        std::string_view _body;

        std::map<std::string_view, std::string_view> _headers;
        std::map<std::string_view, std::string_view> _querys;
        std::map<std::string_view, std::string_view> _contents;
    };
} // namespace Http