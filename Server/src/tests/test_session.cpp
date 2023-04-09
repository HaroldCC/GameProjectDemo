#include <memory.h>
#include <string_view>
#include "Common/include/platform.h"
#include <asio.hpp>
#include "testMsg.pb.h"
#include "Common/net/session.hpp"

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
            std::string_view strContent = std::format("recv message content:{}, id:{}",
                                                      testProtoMsg.msg(), testProtoMsg.id());
            Log::info("{}", strContent);

            std::string        retMsg = std::format("server received message {{}}", strContent);
            net::MessageBuffer buffer(retMsg.size());
            buffer.Write(retMsg);

            AsyncSendMessage(std::move(buffer));
        }
    }
};

class Server
{
public:
    Server(asio::io_context &ioContext, const asio::ip::tcp::endpoint &endpoint)
        : _ioContext(ioContext), _acceptor(_ioContext, endpoint)
    {
        DoAccept();
    }

    // void Start()
    // {
    //     DoAccept();
    // }

private:
    void DoAccept()
    {
        _acceptor.async_accept(
            [this](const std::error_code &errcode, asio::ip::tcp::socket socket)
            {
                if (!errcode)
                {
                    auto pSession = std::make_shared<ServerSession>(std::move(socket));
                    pSession->StartSession();
                }

                DoAccept();
            });
    }

    asio::io_context       &_ioContext;
    asio::ip::tcp::acceptor _acceptor;
};

int main()
{
    asio::io_context ioContext;

    asio::ip::tcp::endpoint endpoint(asio::ip::address_v4().any(), 9988);
    Server                  server(ioContext, endpoint);
    // std::shared_ptr<Server> pServer = std::make_shared<Server>(ioContext, endpoint);

    ioContext.run();

    return 0;
}
