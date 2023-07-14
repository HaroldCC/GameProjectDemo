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

void HttpSession::ReadHandler()
{
    net::MessageBuffer &packet = GetReadBuffer();
    MessageDef::Message message;

    while (packet.ReadableBytes() > 0)
    {
        if (!message.SerializeToArray(packet.GetReadPointer(), (int)packet.ReadableBytes()))
        {
            break;
        }

        const std::string &content = message.content();
    }
}