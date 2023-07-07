#include "session.h"
#include "Common/log.hpp"
#include "Common/include/platform.h"
#include "MessageDef.pb.h"

namespace net
{
    void Session::CloseSession()
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

    void Session::ReadHeader()
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

    void Session::ReadBody()
    {
        auto self(shared_from_this());
        Log::debug("Header:{}", _header);
        _readBuffer.resize(_header);
        asio::async_read(_socket, asio::buffer(_readBuffer),
                         [this, self](const std::error_code &errcode, std::size_t length)
                         {
                             if (errcode)
                             {
                                 CloseSession();
                                 Log::error("读取消息体出错:{}", errcode.message());
                                 return;
                             }

                             MessageDef::Message message;
                             if (message.ParseFromArray(_readBuffer.data(), (int)_readBuffer.size()))
                             {
                                 // todo 处理消息
                                 Log::debug("receive message[header:{}, content:{}]", message.header(), message.content());

                                 std::string response = "Hello Client";
                                 self->SendMessage((uint32_t)response.size(), response);
                             }
                         });
    }

    void Session::SendMessage(uint32_t header, const std::string &message)
    {
        MessageDef::Message send;
        send.set_header(header);
        send.set_content(message);
        MessageBuffer content(send.ByteSizeLong());
        if (send.SerializeToArray(content.data(), (int)content.size()))
        {
            header = asio::detail::socket_ops::host_to_network_long((int)content.size());
            MessageBuffer packet;
            packet.reserve(sizeof(header) + content.size());
            // packet.push_back(*(uint8_t *)&header);
            packet.insert(packet.end(), (uint8_t *)&header, (uint8_t *)&header + sizeof(header));
            packet.insert(packet.cend(), content.begin(), content.end());
            // packet.push_back(content.begin(), content.end());
            _writeBufferQueue.push(packet);

            AsyncWrite();
        }
        else
        {
            Log::error("发送消息出错：【header:{}, content:{}】", header, message);
        }
    }

    void Session::AsyncWrite()
    {
        if (_writeBufferQueue.empty())
        {
            return;
        }

        auto          self(shared_from_this());
        MessageBuffer packet = _writeBufferQueue.front();
        asio::async_write(_socket, asio::buffer(packet),
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