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
#include <vector>

namespace Http
{
    class HttpResponse final
    {
    public:
        /**
         * @brief 获取要发送的Http响应内容
         *
         * @return std::string_view 响应内容
         */
        [[nodiscard]] std::string_view GetPayload();

        void SetStatusCode(Status status);
        void SetHeader(std::string_view fieldName, std::string_view fieldVal);
        // void SetHeader(Field field, std::string_view fieldValue);
        void SetContentType(ContentType type);
        void SetContent(std::string_view content);

        void FillResponse(Status status, ContentType type, std::string_view content);

    private:
        void BuildResponseHead();

    private:
        using ResponseHeader = std::pair<std::string, std::string>;

        Status                      _status      = Status::Unknown;
        ContentType                 _contentType = ContentType::String;
        std::string                 _head;
        std::string                 _content;
        std::vector<ResponseHeader> _headers;
    };
} // namespace Http