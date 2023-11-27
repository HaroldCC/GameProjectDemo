﻿/*************************************************************************
> File Name       : HttpRequest.cpp
> Brief           : http请求
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月16日  20时48分08秒
************************************************************************/
#include "HttpRequest.h"
#include "Common/Util/Log.hpp"

namespace Http
{
    // HttpRequest::HttpRequest()
    // {
    // }

    /**
     * @brief 解析http请求内容
     *
     * @param content http请求内容
     * @return bHttp::status 状态码
     */
    Status HttpRequest::Parse(std::string_view content)
    {
        if (content.empty())
        {
            return Status::BadRequest;
        }

        size_t headerLen = _parser.ParseRequest(content);
        if (headerLen == 0)
        {
            Log::Error("Parser http request error");
            return Status::InternalServerError;
        }

        return Status::Ok;
    }

    [[nodiscard]] std::string_view HttpRequest::GetMethod() const
    {
        return _parser.Method();
    }

    [[nodiscard]] std::string_view HttpRequest::GetPath() const
    {
        return _parser.Path();
    }

    [[nodiscard]] std::string_view HttpRequest::GetVersion() const
    {
        return _parser.MinorVersion() == 1 ? "HTTP/1.1" : "HTTP/1.0";
    }

    [[nodiscard]] std::string_view HttpRequest::GetHeader(std::string_view headerType) const
    {
        return _parser.GetHeaderValue(headerType);
    }

    // [[nodiscard]] std::string_view HttpRequest::GetBody() const
    // {
    //     return _request.body();
    // }

} // namespace Http