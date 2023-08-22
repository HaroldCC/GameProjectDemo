#include "session.h"
#include "Common/include/log.hpp"
#include "Common/include/platform.h"
#include "MessageDef.pb.h"

namespace net
{
    void ISession::CloseSession()
    {
        if (_closed.exchange(true))
        {
            return;
        }

        std::error_code error;
        _socket.shutdown(asio::socket_base::shutdown_send, error);
        if (error)
        {
            Log::Error("shutdown Socket ip:{} error, errorCode:{}, message:{}",
                       GetRemoteIpAddress().to_string(), error.value(), error.message());
        }
    }

    void ISession::StartSession()
    {
        AsyncRead();
    }

    void ISession::AsyncRead()
    {
        auto self(shared_from_this());
        _readBuffer.EnsureFreeSpace();
        _socket.async_read_some(asio::buffer(_readBuffer.GetWritPointer(), _readBuffer.WritableBytes()),
                                [this, self](const std::error_code &errcode, size_t length)
                                {
                                    if (errcode)
                                    {
                                        Log::Error("读取消息出错：{}", errcode.message());
                                        CloseSession();
                                        return;
                                    }

                                    // 更新向buff写入了多少数据
                                    _readBuffer.WriteDone(length);

                                    ReadHandler();

                                    AsyncRead();
                                });
    }

    void ISession::SendMessage(uint32_t header, const std::string &message)
    {
        MessageDef::Message send;
        send.set_header(header);
        Log::Debug("header = {}", header);
        send.set_content(message);
        MessageBuffer content(send.ByteSizeLong());
        if (send.SerializeToArray(content.GetWritPointer(), (int)content.WritableBytes()))
        {
            content.WriteDone(send.ByteSizeLong());
            Log::Debug("send len:{}", content.ReadableBytes());
            _writeBufferQueue.push(std::move(content));

            AsyncWrite();
        }
        else
        {
            Log::Error("发送消息出错：【header:{}, content:{}】", header, message);
        }
    }

    void ISession::AsyncWrite()
    {
        if (_writeBufferQueue.empty())
        {
            return;
        }

        auto           self(shared_from_this());
        MessageBuffer &packet = _writeBufferQueue.front();
        _socket.async_write_some(asio::buffer(packet.GetReadPointer(), packet.ReadableBytes()),
                                 [this, self](const std::error_code &errcode, std::size_t length)
                                 {
                                     if (errcode)
                                     {
                                         CloseSession();
                                         Log::Error("发送消息失败：{}", errcode.message());
                                         return;
                                     }

                                     // 更新从buff读出来多少数据
                                     auto &&buff = _writeBufferQueue.front();
                                     buff.ReadDone(length);

                                     if (buff.ReadableBytes() == 0)
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
                                 });
    }

    void Session::ReadHandler()
    {
        MessageBuffer &packet = GetReadBuffer();
        while (packet.ReadableBytes() > 0)
        {
            MessageDef::Message message;
            if (!message.ParseFromArray(packet.GetReadPointer(), (int)packet.ReadableBytes()))
            {
                break;
            }

            packet.ReadDone(packet.ReadableBytes());

            Log::Debug("receive message[header:{}, content:{}]", message.header(), message.content());

            std::string response = R"(Hello Client
                                    // _header = asio::detail::socket_ops::network_to_host_long(_header);
                                    // _buffer.resize(_header);

                                    // 读取消息体
                                    // asio::async_read(_socket, asio::buffer(_buffer),
                                    //                  [this](const std::error_code &errcode, size_t len)
                                    //                  {
                                    //                      if (errcode)
                                    //                      {
                                    //                          Log::error("读取消息体出错：{}", errcode.message());
                                    //                          return;
                                    //                      }
                                    // _header = asio::detail::socket_ops::network_to_host_long(_header);
                                    // _buffer.resize(_header);

                                    // 读取消息体
                                    // asio::async_read(_socket, asio::buffer(_buffer),
                                    //                  [this](const std::error_code &errcode, size_t len)
                                    //                  {
                                    //                      if (errcode)
                                    //                      {
                                    //                          Log::error("读取消息体出错：{}", errcode.message());
                                    //                          return;
                                    //                      }
                                    // _header = asio::detail::socket_ops::network_to_host_long(_header);
                                    // _buffer.resize(_header);

                                    // 读取消息体
                                    // asio::async_read(_socket, asio::buffer(_buffer),
                                    //                  [this](const std::error_code &errcode, size_t len)
                                    //                  {
                                    //                      if (errcode)
                                    //                      {
                                    //                          Log::error("读取消息体出错：{}", errcode.message());
                                    //                          return;
                                    //                      }
                                    // _header = asio::detail::socket_ops::network_to_host_long(_header);
                                    // _buffer.resize(_header);

                                    // 读取消息体
                                    // asio::async_read(_socket, asio::buffer(_buffer),
                                    //                  [this](const std::error_code &errcode, size_t len)
                                    //                  {
                                    //                      if (errcode)
                                    //                      {
                                    //                          Log::error("读取消息体出错：{}", errcode.message());
                                    //                          return;
                                    //                      }
                                    // _header = asio::detail::socket_ops::network_to_host_long(_header);
                                    // _buffer.resize(_header);

                                    // 读取消息体
                                    // asio::async_read(_socket, asio::buffer(_buffer),
                                    //                  [this](const std::error_code &errcode, size_t len)
                                    //                  {
                                    //                      if (errcode)
                                    //                      {
                                    //                          Log::error("读取消息体出错：{}", errcode.message());
                                    //                          return;
                                    //                      }
                                    // _header = asio::detail::socket_ops::network_to_host_long(_header);
                                    // _buffer.resize(_header);

                                    // 读取消息体
                                    // asio::async_read(_socket, asio::buffer(_buffer),
                                    //                  [this](const std::error_code &errcode, size_t len)
                                    //                  {
                                    //                      if (errcode)
                                    //                      {
                                    //                          Log::error("读取消息体出错：{}", errcode.message());
                                    //                          return;
                                    //                      }
                                    // _header = asio::detail::socket_ops::network_to_host_long(_header);
                                    // _buffer.resize(_header);

                                    // 读取消息体
                                    // asio::async_read(_socket, asio::buffer(_buffer),
                                    //                  [this](const std::error_code &errcode, size_t len)
                                    //                  {
                                    //                      if (errcode)
                                    //                      {
                                    //                          Log::error("读取消息体出错：{}", errcode.message());
                                    //                          return;
                                    //                      }
                        )";
            SendMessage(response.size(), response);
        }
    }

} // namespace net