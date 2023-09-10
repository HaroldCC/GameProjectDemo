/*************************************************************************
> File Name       : HttpResponse.h
> Brief           : Http响应
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月17日  12时21分05秒
************************************************************************/
#pragma once
#include "boost/beast.hpp"

namespace bHttp = boost::beast::http;

namespace Http
{
    using status = boost::beast::http::status;
    using field  = boost::beast::http::field;

    class HttpResponse final
    {
    public:
        /**
         * @brief 获取要发送的Http响应内容
         *
         * @return std::string_view 响应内容
         */
        [[nodiscard]] std::string_view GetPayload() const;

        void SetStatusCode(status status);
        void SetHeader(std::string_view fieldName, std::string_view fieldVal);
        void SetHeader(field field, std::string_view fieldValue);
        void SetContent(std::string_view content);

    private:
        bHttp::response<bHttp::string_body> _response;
    };
} // namespace Http