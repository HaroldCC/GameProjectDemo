#include <memory.h>
#include "Common/include/platform.h"
#include <asio.hpp>
#include "testMsg.pb.h"

import net;
import common;

class ServerSession : public net::Session
{
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
            // Proto::TestMsg testProtoMsg;
            // testProtoMsg.ParsePartialFromArray(buffer.Data(), buffer.ReadableBytes());
            // std::string_view content = std::format("recv message content:{}, id:{}",
            //                                        testProtoMsg.msg(), testProtoMsg.id());
            // Log::info(content);

            // std::string        retMsg = std::format("server received message {{}}", content);
            // net::MessageBuffer buffer(retMsg.size());
            // buffer.Write(retMsg);

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
        _acceptor.async_accept([this, self](const std::error_code &error, const asio::ip::tcp::socket &socket)
                               {
                                   if (!error)
                                   {
                                    auto session = std::make_shared<net::Session>(std::move(socket)) ;
                                       session->StartSession();
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
    Server(ioContext, endpoint);

    ioContext.run();

    return 0;
}