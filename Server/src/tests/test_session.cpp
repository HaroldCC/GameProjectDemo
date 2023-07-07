﻿#include <memory.h>
#include <string_view>
#include "Common/include/platform.h"
#include <asio.hpp>
#include "testMsg.pb.h"
#include "net/session.hpp"

class ServerSession : public net::Session
{
public:
    explicit ServerSession(asio::ip::tcp::socket &&socket)
        : Session(std::move(socket))
    {
    }

protected:
    void ReadHandler() override
    {
        if (!IsAlive())
        {
            return;
        }

        net::MessageBuffer &buffer = GetReadBuffer();
        while (buffer.ReadableBytes() > 0)
        {
            Proto::TestMsg testProtoMsg;
            testProtoMsg.ParsePartialFromArray(buffer.Data(), (int)buffer.ReadableBytes());
            std::string strContent = std::format("recv message content:{}, id:{}",
                                                 testProtoMsg.msg(), testProtoMsg.id());
            Log::info("{}", strContent);

            std::string        retMsg = std::format("server received message {{}}", strContent);
            net::MessageBuffer buffer(retMsg.size());
            buffer.Write(retMsg);

            AsyncSendMessage(std::move(buffer));
        }
    }
};

class Server : std::enable_shared_from_this<Server>
{
public:
    Server(asio::io_context &ioContext, const asio::ip::tcp::endpoint &endpoint)
        : _ioContext(ioContext), _acceptor(_ioContext, endpoint)
    {
        DoAccept();
    }

protected:
private:
    void DoAccept()
    {
        auto self(shared_from_this());
        _acceptor.async_accept([this, self](const std::error_code &error, asio::ip::tcp::socket socket)
                               {
                                   if (!error)
                                   {
                                    auto pSession = std::make_shared<ServerSession>(std::move(socket)) ;
                                       pSession->StartSession();
                                   }

                                   DoAccept(); });
    }

    asio::io_context       &_ioContext;
    asio::ip::tcp::acceptor _acceptor;
};

int main()
{
    asio::io_context ioContext;

    asio::ip::tcp::endpoint endpoint(asio::ip::address_v4().any(), 9988);
    Server                  server(ioContext, endpoint);

    ioContext.run();

    return 0;
}