module;

#include <memory>
#include <queue>
#include <atomic>

#include "Common/include/platform.h"
#include "spdlog/spdlog.h"
#include <asio.hpp>

export module net:session;
import common;
import :buffer;

export namespace net
{
    class Session : public std::enable_shared_from_this<Session>
    {
    public:
        explicit Session(asio::ip::tcp::socket &&socket)
            : _socket(std::move(socket)), _remoteAddress(_socket.remote_endpoint().address()),
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
            AsyncRead();
        }

        void AsyncSendMessage(MessageBuffer &&buffer)
        {
            _writeBufferQueue.push(std::move(buffer));

            AsyncWrite();
        }

        void CloseSession()
        {
            if (_closed.exchange(true))
            {
                return;
            }

            std::error_code error;
            _socket.shutdown(asio::socket_base::shutdown_send, error);
            if (error)
            {
                Log::error("shutdown Socket ip:{} error, errorcode:{}, message:{}",
                           GetRemoteIpAddress().to_string(), error.value(), error.message());
            }

            // OnClose();
        }

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

    private:
        void AsyncRead()
        {
            if (!IsAlive())
            {
                return;
            }

            auto self(shared_from_this());
            _socket.async_read_some(asio::buffer(_readBuffer.Data(), _readBuffer.WritableBytes()),
                                    [this, self](const std::error_code &ec, std::size_t length)
                                    {
                                        if (ec)
                                        {
                                            CloseSession();
                                            return;
                                        }

                                        ReadHandler();
                                    });
        }

        void AsyncWrite()
        {
            auto           self(shared_from_this());
            MessageBuffer &buffer = _writeBufferQueue.front();
            _socket.async_write_some(asio::buffer(buffer.Data(), buffer.ReadableBytes()),
                                     [this, self, &buffer](const std::error_code &ec, std::size_t length)
                                     {
                                         if (ec)
                                         {
                                             CloseSession();
                                         }
                                         else
                                         {
                                             if (buffer.ReadableBytes() <= 0)
                                             {
                                                 _writeBufferQueue.pop();
                                             }

                                             if (!_writeBufferQueue.empty())
                                             {
                                                 AsyncWrite();
                                             }
                                             else if (_closing)
                                             {
                                                 CloseSession();
                                             }
                                         }
                                     });
        }

        asio::ip::tcp::socket     _socket;
        asio::ip::address         _remoteAddress;
        uint16_t                  _remotePort;
        uint32_t                  _header;
        MessageBuffer             _readBuffer;
        std::queue<MessageBuffer> _writeBufferQueue;

        std::atomic_bool _closed;
        std::atomic_bool _closing;
    };
}