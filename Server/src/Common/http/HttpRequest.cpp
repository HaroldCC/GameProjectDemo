/*************************************************************************
> File Name       : HttpRequest.cpp
> Brief           : http请求
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月16日  20时48分08秒
************************************************************************/
#include "HttpRequest.h"
#include "include/Util.hpp"
#include "Common/include/Log.hpp"
#include "boost/beast.hpp"

namespace Http
{
    struct HttpRequest::HttpRequestImpl
    {
        boost::beast::http::request<boost::beast::http::string_body> _request;
    };

    HttpRequest::HttpRequest()
        : _pImpl(std::make_shared<HttpRequestImpl>())
    {
    }

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
            return Status::bad_request;
        }

        // 创建请求解析，并设置立即解析
        boost::beast::http::request_parser<boost::beast::http::string_body> requestParser;
        requestParser.eager(true);

        boost::beast::error_code errcode;
        requestParser.put(boost::asio::buffer(content), errcode);
        if (errcode)
        {
            Log::Error("Parser http request error:{}", errcode.message());
            return Status::internal_server_error;
        }

        _pImpl->_request = requestParser.release();

        return Status::ok;
    }

    [[nodiscard]] std::string_view HttpRequest::GetMethod() const
    {
        return _pImpl->_request.method_string();
    }

    [[nodiscard]] std::string_view HttpRequest::GetPath() const
    {
        return _pImpl->_request.target();
    }

    [[nodiscard]] std::string_view HttpRequest::GetVersion() const
    {
        return _pImpl->_request.version() == 11 ? "HTTP/1.1" : "HTTP/1.0";
    }

    [[nodiscard]] std::string_view HttpRequest::GetHeader(std::string_view headerType) const
    {
        return _pImpl->_request[headerType];
    }

    // [[nodiscard]] std::string_view HttpRequest::GetBody() const
    // {
    //     return _request.body();
    // }

} // namespace Http