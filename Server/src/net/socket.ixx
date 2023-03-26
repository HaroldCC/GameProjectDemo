module;

#include <memory>
#include <queue>
#include <atomic>

#include "include/platform.h"
#include <asio/ip/tcp.hpp>

export module net:socket;
import :buffer;

export namespace net
{
    template <typename T>
    class Socket : public std::enable_shared_from_this<T>
    {
    public:
        explicit Socket(asio::ip::tcp::socket &&socket)
            : _socket(socket), _remoteAddress(_socket.remote_endpoint().address()),
              _remotePort(_socket.remote_endpoint().port()),
              _recvBuffer(),
              _closed(false), _closing(false), _isSendingSync(false)
        {
        }

        virtual ~Socket()
        {
            _closed = true;
            std::error_code errcode;
            _socket.close(errcode);
        }

        virtual void Connected() = 0;

        virtual bool Update()
        {
            if (_closed)
            {
                return false;
            }

            while ()
        }

    private:
        bool HandleQueue()
        {
        }

        asio::ip::tcp::socket _socket;
        asio::ip::address     _remoteAddress;
        uint16_t              _remotePort;
        Buffer                _recvBuffer;
        std::queue<Buffer>    _sendQueue;

        std::atomic_bool _closed;
        std::atomic_bool _closing;
        bool             _isSendingSync;
    };
}