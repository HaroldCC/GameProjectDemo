/*************************************************************************
> File Name       : HttpRequest.cpp
> Brief           : http请求
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月16日  20时48分08秒
************************************************************************/
#include "pch.h"
#include "HttpRequest.h"
#include "include/util.hpp"
#include "Common/include/log.hpp"

namespace Http
{
    /**
     * @brief 解析http请求内容
     *
     * @param content http请求内容
     * @return bHttp::status 状态码
     */
    bHttp::status HttpRequest::Parse(std::string_view content)
    {
        if (content.empty())
        {
            return bHttp::status::bad_request;
        }

        // 创建请求解析，并设置立即解析
        bHttp::request_parser<bHttp::string_body> requestParser;
        requestParser.eager(true);

        boost::beast::error_code errcode;
        requestParser.put(boost::asio::buffer(content), errcode);
        if (errcode)
        {
            Log::Error("Parser http request error:{}", errcode.message());
            return bHttp::status::internal_server_error;
        }

        _request = requestParser.release();

        return bHttp::status::ok;
    }

    [[nodiscard]] std::string_view HttpRequest::GetMethod() const
    {
        return _request.method_string();
    }

    [[nodiscard]] std::string_view HttpRequest::GetPath() const
    {
        return _request.target();
    }

    [[nodiscard]] std::string_view HttpRequest::GetVersion() const
    {
        return _request.version() == 11 ? "HTTP/1.1" : "HTTP/1.0";
    }

    [[nodiscard]] std::string_view HttpRequest::GetHeader(std::string_view headerType) const
    {
        return _request[headerType];
    }

    // [[nodiscard]] std::string_view HttpRequest::GetBody() const
    // {
    //     return _request.body();
    // }

} // namespace Http