/*************************************************************************
> File Name       : HttpSession.cpp
> Brief           : http会话
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月13日  20时03分26秒
************************************************************************/
#include "HttpSession.h"

#include "NetMessage.pb.h"
#include "Common/Util/Log.hpp"

namespace Http
{
    void HttpSession::AddRouter(HttpMethod method, std::string_view path, HttpHandlerFunc handler)
    {
        _router.AddHttpHandler(method, path, std::move(handler));
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
            _router.Route(_req, _rep);

            std::string_view   response = _rep.GetPayload();
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