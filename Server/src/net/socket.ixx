module;
#include <memory>
#include <queue>
#include <atomic>

#include "include/platform.h"
#include <asio/ip/tcp.hpp>

import common;
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

            while (HandleQueue())
                ;
        }

        void CloseSocket()
        {
            if (_closed.exchange(true))
            {
                return;
            }

            std::error_code error;
            _socket.shutdown(asio::socket_base::shutdown_send, error);
            if (error)
            {
                logger::error("shutdown Socket ip:{} error, errorcode:{}, message:{}", GetRemoteIpAddress().to_string(), error.value(), error.message());
            }

            OnClose();
        }

        asio::ip::address GetRemoteIpAddress()
        {
            return _remoteAddress;
        }
    protected:
        virtual void OnClose() {}

        bool AsyncProcessQueue()
        {
            if (_isSendingSync)
                return false;

            _isSendingSync = true;
            _socket.async_write_some(asio::null_buffers(), std::bind(&Socket<T>::SendHandler, this->shared_from_this(), std::placeholders::_1, std::placeholders::_2));
            return false;
        }

    private:
        bool HandleQueue()
        {
            if (_sendQueue.empty())
                return false;

            Buffer buffer = _sendQueue.front();
            size_t readSize   = buffer.ReadableBytes();

            std::error_code error;
            size_t          byteSent = _socket.write_some(asio::buffer(buffer.Peek(), readSize), error);
            if (error)
            {
                if (error == std::errc::operation_would_block || error == std::errc::resource_unavailable_try_again)
                {
                    return AsyncProcessQueue();
                }

                _sendQueue.pop();
                if (_closing && _sendQueue.empty())
                {
                    CloseSocket();
                }
            }
        }

        void SendHandler(std::error_code error, size_t sendBytes)
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