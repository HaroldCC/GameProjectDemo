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
            Log::error("shutdown Socket ip:{} error, errorCode:{}, message:{}",
                       GetRemoteIpAddress().to_string(), error.value(), error.message());
        }
    }

    void ISession::ReadHeader()
    {
        auto self(shared_from_this());
        asio::async_read(_socket, asio::buffer(&_header, sizeof(_header)),
                         [this, self](const std::error_code &errcode, std::size_t length)
                         {
                             if (errcode)
                             {
                                 Log::error("读取消息头出错：{}", errcode.message());
                                 CloseSession();
                                 return;
                             }

                             _header = asio::detail::socket_ops::network_to_host_long(_header);
                             ReadBody();
                         });
    }

    void ISession::ReadBody()
    {
        auto self(shared_from_this());
        Log::debug("Header:{}", _header);
        _readBuffer.EnsureWritableBytes(_header);
        // todo buff 空间问题
        asio::async_read(_socket, asio::buffer(_readBuffer.GetWritPointer(), _readBuffer.WritableBytes()),
                         [this, self](const std::error_code &errcode, std::size_t length)
                         {
                             if (errcode)
                             {
                                 CloseSession();
                                 Log::error("读取消息体出错:{}", errcode.message());
                                 return;
                             }

                             if (_readBuffer.ReadableBytes() >= _header)
                             {
                                 MessageDef::Message message;
                                 if (message.ParseFromArray(_readBuffer.GetReadPointer(), (int)_readBuffer.ReadableBytes()))
                                 {
                                     // todo 处理消息
                                     Log::debug("receive message[header:{}, content:{}]", message.header(), message.content());

                                     std::string response = "Hello Client";
                                     self->SendMessage((uint32_t)response.size(), response);
                                 }
                             }
                         });
    }

    void ISession::SendMessage(uint32_t header, const std::string &message)
    {
        MessageDef::Message send;
        send.set_header(header);
        send.set_content(message);
        MessageBuffer content(send.ByteSizeLong());
        if (send.SerializeToArray(content.GetReadPointer(), (int)content.ReadableBytes()))
        {
            header = asio::detail::socket_ops::host_to_network_long((int)content.ReadableBytes());
            MessageBuffer packet(sizeof(header) + content.ReadableBytes());
            // packet.reserve(sizeof(header) + content.size());
            // packet.insert(packet.end(), (uint8_t *)&header, (uint8_t *)&header + sizeof(header));
            // packet.insert(packet.cend(), content.begin(), content.end());
            packet.Write(&header, sizeof(header));
            packet.Write(content.GetReadPointer(), content.ReadableBytes());
            _writeBufferQueue.push(packet);

            AsyncWrite();
        }
        else
        {
            Log::error("发送消息出错：【header:{}, content:{}】", header, message);
        }
    }

    void ISession::AsyncWrite()
    {
        if (_writeBufferQueue.empty())
        {
            return;
        }

        auto          self(shared_from_this());
        MessageBuffer packet = _writeBufferQueue.front();
        asio::async_write(_socket, asio::buffer(packet.GetReadPointer(), packet.ReadableBytes()),
                          [this, self](const std::error_code &errcode, std::size_t length)
                          {
                              if (errcode)
                              {
                                  CloseSession();
                                  Log::error("发送消息失败：{}", errcode.message());
                              }
                              else
                              {
                                  _writeBufferQueue.pop();

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

} // namespace net