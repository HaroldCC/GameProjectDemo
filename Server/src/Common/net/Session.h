/*************************************************************************
> File Name       : session.h
> Brief           : 网络会话
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年04月04日  14时27分01秒
************************************************************************/
#pragma once

#include <memory>
#include <queue>
#include <atomic>
#include <asio.hpp>
#include "buffer.hpp"

// #include "buffer.hpp"

namespace net
{
    class ISession : public std::enable_shared_from_this<ISession>
    {
    public:
        ISession(const ISession &)            = delete;
        ISession(ISession &&)                 = delete;
        ISession &operator=(const ISession &) = delete;
        ISession &operator=(ISession &&)      = delete;
        explicit ISession(asio::ip::tcp::socket &&socket)
            : _socket(std::move(socket)),
              _remoteAddress(_socket.remote_endpoint().address()),
              _remotePort(_socket.remote_endpoint().port()),
              _closed(false), _closing(false)
        {
        }

        virtual ~ISession()
        {
            if (_closed)
            {
                return;
            }

            std::error_code errcode;
            _socket.close(errcode);
        }

        void StartSession();

        void SendProtoMessage(size_t header, const std::string &message);
        void SendMsg(const net::MessageBuffer &message);

        void CloseSession();

        asio::ip::address GetRemoteIpAddress()
        {
            return _remoteAddress;
        }

        bool IsAlive()
        {
            return !_closed && !_closing;
        }

        void DelayedCloseSession()
        {
            _closing = true;
        }

        MessageBuffer &GetReadBuffer()
        {
            return _readBuffer;
        }

    protected:
        virtual void ReadHandler() = 0;

        template <typename Proto>
        std::optional<Proto> BufferToProto(MessageBuffer &message)
        {
            Proto proto;
            if (!proto.ParseFromArray(message.GetReadPointer(), (int)message.ReadableBytes()))
            {
                return std::nullopt;
            }
            message.ReadDone(message.ReadableBytes());

            return proto;
        }

        template <typename Proto>
        std::optional<MessageBuffer> ProtoToBuffer(const Proto &proto)
        {
            auto          size = proto.ByteSizeLong();
            MessageBuffer message(size);
            if (!proto.SerializeToArray(message.GetWritPointer(), (int)size))
            {
                return std::nullopt;
            }
            message.WriteDone(size);

            return message;
        }

        void AsyncRead();

        void AsyncWrite();

    private:
        asio::ip::tcp::socket _socket;
        asio::ip::address     _remoteAddress;
        uint16_t              _remotePort;

        uint32_t                  _header{0};
        MessageBuffer             _readBuffer;
        std::queue<MessageBuffer> _writeBufferQueue;

        std::atomic_bool _closed;
        std::atomic_bool _closing;
    };

    class Session : public ISession
    {
    public:
        using ISession::ISession;

    protected:
        void ReadHandler() override;
    };
} // namespace net