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

#include "MessageDef.pb.h"
#include "Common/include/Log.hpp"

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

        const std::string &content = packet.ReadAllAsString();
        if (!content.empty())
        {
            Log::Info("Http:{}", content);
            _req.Parse(content);
            if (auto rep = _router.Handle(_req); rep.has_value())
            {
                _rep = rep.value();
            }
            else
            {
                _rep.SetStatusCode(status::not_found);
                _rep.SetContent(std::format("Not Found {}", _req.GetPath()));
            }

            std::string_view response = _rep.GetPayload();
            Log::Info("http Response:{}", response);
            net::MessageBuffer sendBuffer(response.size());
            sendBuffer.Write(response);
            SendMsg(sendBuffer);
            // SendProtoMessage((uint32_t)response.size(), response.data());
        }
        else
        {
            Log::Error("解析Http请求内容出错");
        }
    }
} // namespace Http