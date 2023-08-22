/*************************************************************************
> File Name       : HttpResponse.cpp
> Brief           : Http响应
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月17日  14时42分28秒
************************************************************************/
#include "pch.h"
#include "HttpResponse.h"
#include "Common/include/Log.hpp"

namespace Http
{

    /**
     * @brief 获取要发送的Http响应内容
     *
     * @return std::string_view 响应内容
     */
    [[nodiscard]] std::string_view HttpResponse::GetPayload() const
    {
        std::ostringstream oss;
        oss << _response;

        return oss.view();
    }

    void HttpResponse::SetStatusCode(status status)
    {
        _response.result(status);
    }

    void HttpResponse::SetHeader(std::string_view fieldName, std::string_view fieldVal)
    {
        try
        {
            _response.set(fieldName, fieldName);
        }
        catch (const std::exception &e)
        {
            Log::Error("Http set header error:{}", e.what());
        }
    }

    void HttpResponse::SetHeader(field field, std::string_view fieldVal)
    {
        try
        {
            _response.set(field, fieldVal);
        }
        catch (const std::exception &e)
        {
            Log::Error("Http set header error:{}", e.what());
        }
    }

    void HttpResponse::SetBody(std::string_view body)
    {
        try
        {
            _response.set(field::body, body);
        }
        catch (const std::exception &e)
        {
            Log::Error("Http set body error:{}", e.what());
        }
    }
} // namespace Http