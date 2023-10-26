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
#include "Common/include/Util.hpp"
#include "Common/include/TimeUtil.hpp"
#include "HttpUtil.hpp"
#include <algorithm>

namespace Http
{
    /**
     * @brief 获取要发送的Http响应内容
     *
     * @return std::string_view 响应内容
     */
    [[nodiscard]] std::string_view HttpResponse::GetPayload()
    {
        BuildResponseHead();
        _head.append(std::format("{}\r\n{}", ContentTypeString(_contentType), _content));
        return _head;
    }

    void HttpResponse::SetStatusCode(Status status)
    {
        _status = status;
    }

    void HttpResponse::SetHeader(std::string_view fieldName, std::string_view fieldVal)
    {
        _headers.emplace_back(fieldName, fieldVal);
    }

    // void HttpResponse::SetHeader(Field field, std::string_view fieldVal)
    // {
    //     _headers.emplace_back()
    // }

    void HttpResponse::SetContentType(ContentType type)
    {
        _contentType = type;
    }

    void HttpResponse::SetContent(std::string_view content)
    {
        _content = content;
    }

    void HttpResponse::FillResponse(Status status, ContentType type, std::string_view content)
    {
        _status      = status;
        _contentType = type;
        _content     = content;
    }

    void HttpResponse::BuildResponseHead()
    {
        if (std::find_if(_headers.begin(),
                         _headers.end(),
                         [](ResponseHeader &header) {
                             return header.first == "Host";
                         })
            == _headers.end())
        {
            _headers.emplace_back("Host", "Harold");
        }

        if (_status >= Status::NotFound)
        {
            _content.append(StatusToResponseContent(_status));
        }

        if (_content.empty())
        {
            _headers.emplace_back("Content-Length", "0");
        }
        else
        {
            _headers.emplace_back("Content-Length", Util::ToString(_content.size()));
        }

        _headers.emplace_back("Date", Util::TimeUtil::GetGmtTimeStr());

        _head.append(StatusToResponseHead(_status));
        for (auto &[k, v] : _headers)
        {
            _head.append(std::format("{}:{}\r\n", k, v));
        }
    }
} // namespace Http