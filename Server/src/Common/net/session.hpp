﻿/*************************************************************************
> File Name       : session.h
> Brief           : 网络会话
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年04月04日  14时27分01秒
************************************************************************/
#pragma once

#include "Common/pch.h"
#include "buffer.hpp"
#include "Common/log.hpp"

namespace net
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

            // std::error_code errcode;
            // _socket.close(errcode);
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
            _socket.async_read_some(asio::buffer(_readBuffer.GetWritPointer(), 1024),
                                    [this, self](const std::error_code &ec, std::size_t length)
                                    {
                                        if (ec)
                                        {
                                            CloseSession();
                                            return;
                                        }

                                        // _readBuffer.WriteInBytes(length);
                                        _readBuffer.WriteDone(length);

                                        ReadHandler();
                                    });
        }

        void AsyncWrite()
        {
            auto           self(shared_from_this());
            MessageBuffer &buffer = _writeBufferQueue.front();
            _socket.async_write_some(asio::buffer(buffer.GetReadPointer(), buffer.ReadableBytes()),
                                     [this, self, &buffer](const std::error_code &ec, std::size_t length)
                                     {
                                         //  buffer.ReadOutBytes(length);
                                         //  buffer.Read(buffer.Data(), length);
                                         buffer.ReadDone(length);

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
        MessageBuffer             _readBuffer;
        std::queue<MessageBuffer> _writeBufferQueue;

        std::atomic_bool _closed;
        std::atomic_bool _closing;
    };
} // namespace net