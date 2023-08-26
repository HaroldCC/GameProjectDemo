/*************************************************************************
> File Name       : HttpSession.cpp
> Brief           : http会话
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月13日  20时03分26秒
************************************************************************/
#include "pch.h"
#include "HttpSession.h"

#include <utility>
#include "MessageDef.pb.h"

namespace Http
{
    void HttpSession::AddRouter(boost::beast::http::verb method, std::string_view path, HttpHandlerFunc handler)
    {
        _router.AddRouter(method, path, std::move(handler));
    }

    void HttpSession::ReadHandler()
    {
        net::MessageBuffer &packet = GetReadBuffer();
        if (packet.ReadableBytes() <= 0)
        {
            return;
        }

        if (auto message = BufferToProto<MessageDef::Message>(packet); message.has_value())
        {
            const std::string &content = message.value().content();
            _req.Parse(content);
            if (auto rep = _router.Handle(_req); rep.has_value())
            {
                _rep = rep.value();
            }
            else
            {
                _rep.SetStatusCode(status::not_found);
                _rep.SetBody(std::format("Not Found {}", _req.GetPath()));
            }
        }

        std::string_view response = _rep.GetPayload();
        SendMessage((uint32_t)response.size(), response.data());
    }
} // namespace Http