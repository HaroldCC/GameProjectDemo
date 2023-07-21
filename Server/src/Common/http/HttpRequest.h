/*************************************************************************
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
#include "boost/beast.hpp"

namespace bHttp = boost::beast::http;

namespace Http
{
    class HttpRequest final
    {
    public:
        HttpRequest()                               = default;
        HttpRequest(HttpRequest &&)                 = default;
        HttpRequest &operator=(HttpRequest &&)      = default;
        HttpRequest(const HttpRequest &)            = default;
        HttpRequest &operator=(const HttpRequest &) = default;
        ~HttpRequest()                              = default;

        /**
         * @brief 解析http请求内容
         *
         * @param content http请求内容
         * @return bHttp::status 状态码
         */
        bHttp::status Parse(std::string_view content);

        [[nodiscard]] std::string_view GetMethod() const;
        [[nodiscard]] std::string_view GetPath() const;
        [[nodiscard]] std::string_view GetVersion() const;
        [[nodiscard]] std::string_view GetHeader(std::string_view headerType) const;
        [[nodiscard]] std::string_view GetBody() const;

    private:
        bHttp::request<bHttp::string_body> _request;
    };
} // namespace Http