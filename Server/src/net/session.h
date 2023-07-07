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

// #include "buffer.hpp"

namespace net
{
    class Session final : public std::enable_shared_from_this<Session>
    {
        using MessageBuffer = std::vector<uint8_t>;

    public:
        Session(const Session &)            = delete;
        Session(Session &&)                 = delete;
        Session &operator=(const Session &) = delete;
        Session &operator=(Session &&)      = delete;
        explicit Session(asio::ip::tcp::socket &&socket)
            : _socket(std::move(socket)),
              _remoteAddress(_socket.remote_endpoint().address()),
              _remotePort(_socket.remote_endpoint().port()),
              _closed(false), _closing(false)
        {
        }

        virtual ~Session()
        {
            if (_closed)
            {
                return;
            }

            std::error_code errcode;
            _socket.close(errcode);
        }

        void StartSession()
        {
            ReadHeader();
        }

        void SendMessage(uint32_t header, const std::string &message);

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

    private:
        void ReadHeader();

        void ReadBody();

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
} // namespace net