/*************************************************************************
> File Name       : HttpRequest.h
> Brief           : http请求
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月13日  19时45分54秒
************************************************************************/
#pragma once
#include <string_view>
#include "HttpCommon.h"
#include "HttpParser.h"

namespace Http
{
    class HttpRequest final
    {
    public:
        /**
         * @brief 解析http请求内容
         *
         * @param content http请求内容
         * @return bHttp::status 状态码
         */
        Status Parse(std::string_view content);

        [[nodiscard]] std::string_view GetMethod() const;
        [[nodiscard]] std::string_view GetPath() const;
        [[nodiscard]] std::string_view GetVersion() const;
        [[nodiscard]] std::string_view GetHeader(std::string_view headerType) const;
        [[nodiscard]] std::string_view GetBody() const;

    private:
        HttpParser _parser;
    };
} // namespace Http