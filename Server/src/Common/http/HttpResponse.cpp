/*************************************************************************
> File Name       : HttpResponse.cpp
> Brief           : Http响应
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月17日  14时42分28秒
************************************************************************/
#include "HttpResponse.h"
#include "Common/include/Log.hpp"
#include "boost/beast.hpp"

namespace Http
{
    struct HttpResponse::HttpResponseImpl
    {
        boost::beast::http::response<boost::beast::http::string_body> _response;
    };

    HttpResponse::HttpResponse()
        : _pImpl(std::make_shared<HttpResponseImpl>())
    {
    }

    /**
     * @brief 获取要发送的Http响应内容
     *
     * @return std::string_view 响应内容
     */
    [[nodiscard]] std::string_view HttpResponse::GetPayload() const
    {
        std::ostringstream oss;
        oss << _pImpl->_response;

        return oss.view();
    }

    void HttpResponse::SetStatusCode(Status status)
    {
        _pImpl->_response.result(static_cast<boost::beast::http::status>(status));
    }

    void HttpResponse::SetHeader(std::string_view fieldName, std::string_view fieldVal)
    {
        try
        {
            _pImpl->_response.set(fieldName, fieldName);
        }
        catch (const std::exception &e)
        {
            Log::Error("Http set header error:{}", e.what());
        }
    }

    void HttpResponse::SetHeader(Field field, std::string_view fieldVal)
    {
        try
        {
            _pImpl->_response.set(static_cast<boost::beast::http::field>(field), fieldVal);
        }
        catch (const std::exception &e)
        {
            Log::Error("Http set header error:{}", e.what());
        }
    }

    void HttpResponse::SetContent(std::string_view content)
    {
        try
        {
            size_t length = content.size();
            _pImpl->_response.set(static_cast<boost::beast::http::field>(Field::content_length), std::format("{}\r\n\r\n{}", length, content));
        }
        catch (const std::exception &e)
        {
            Log::Error("Http set body error:{}", e.what());
        }
    }
} // namespace Http