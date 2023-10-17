/*************************************************************************
> File Name       : HttpResponse.h
> Brief           : Http响应
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月17日  12时21分05秒
************************************************************************/
#pragma once
#include "HttpCommon.h"
#include <string_view>
#include <memory>

namespace Http
{
    class HttpResponse final
    {
    public:
        HttpResponse();

        /**
         * @brief 获取要发送的Http响应内容
         *
         * @return std::string_view 响应内容
         */
        [[nodiscard]] std::string_view GetPayload() const;

        void SetStatusCode(Status status);
        void SetHeader(std::string_view fieldName, std::string_view fieldVal);
        void SetHeader(Field field, std::string_view fieldValue);
        void SetContent(std::string_view content);

    private:
        struct HttpResponseImpl;
        std::shared_ptr<HttpResponseImpl> _pImpl;
    };
} // namespace Http