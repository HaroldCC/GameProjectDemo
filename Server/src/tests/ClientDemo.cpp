#include "Common/include/platform.h"
#include "Common/net/server.h"
#include "MessageDef.pb.h"
#include "Common/include/log.hpp"
#include "Common/net/buffer.hpp"

using asio::ip::address;
using asio::ip::tcp;

class Client
{
public:
    Client(asio::io_context &ioContext, tcp::endpoint &endpoint)
        : _ioContext(ioContext), _socket(ioContext), _endPoint(endpoint)
    {
    }

    void Start()
    {
        _socket.async_connect(_endPoint,
                              [this](const std::error_code &errcode)
                              {
                                  if (errcode)
                                  {
                                      Log::error("连接服务器失败：{}", errcode.message());
                                      return;
                                  }

                                  SendMessage();
                              });
    }

    void Close()
    {
        std::error_code error;
        _socket.shutdown(asio::socket_base::shutdown_receive, error);
    }

    void ReadMessage()
    {
        _buffer.EnsureFreeSpace();
        _socket.async_read_some(asio::buffer(_buffer.GetWritPointer(), _buffer.WritableBytes()),
                                [this](const std::error_code &errcode, size_t len)
                                {
                                    if (errcode)
                                    {
                                        Log::error("读取消息头出错：{}", errcode.message());
                                        return;
                                    }
                                    _buffer.WriteDone(len);

                                    if (_buffer.ReadableBytes() <= 0)
                                    {
                                        Log::debug("nothing to read");
                                    }

                                    MessageDef::Message message;
                                    if (message.ParseFromArray(_buffer.GetReadPointer(), (int)_buffer.ReadableBytes()))
                                    {
                                        Log::debug("接收到消息【header:{}, content:{}】", message.header(), message.content());
                                        _buffer.ReadDone(_buffer.ReadableBytes());
                                        Close();
                                        return;
                                    }

                                    ReadMessage();
                                });
    }

    void SendMessage()
    {
        MessageDef::Message message;
        std::string         strTest = R"(Hello Server
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
                                    //                      } end.
                                    )";
        message.set_content(strTest);
        message.set_header(message.ByteSizeLong());
        Log::debug("header = {}", message.header());

        std::vector<uint8_t> content(message.ByteSizeLong());
        Log::debug("message size:{}", message.ByteSizeLong());
        if (message.SerializeToArray(content.data(), (int)content.size()))
        {
            uint32_t           header = asio::detail::socket_ops::host_to_network_long((int)content.size());
            net::MessageBuffer buffSend(content.size());
            buffSend.Write(content.data(), content.size());
            Log::debug("buff size:{}", buffSend.ReadableBytes());
            asio::async_write(_socket, asio::buffer(buffSend.GetReadPointer(), buffSend.ReadableBytes()),
                              [this, &buffSend](const std::error_code &errcode, std::size_t len)
                              {
                                  if (errcode)
                                  {
                                      Log::error("发送消息失败：{}", errcode.message());
                                      return;
                                  }

                                  Log::debug("send len: {}", len);
                                  buffSend.ReadDone(len);

                                  //  if (len <)

                                  ReadMessage();
                              });
        }
    }

private:
    asio::io_context  &_ioContext;
    tcp::socket        _socket;
    tcp::endpoint      _endPoint;
    net::MessageBuffer _buffer;
};

int main()
{
    Log::CLogger::GetLogger().InitLogger("log/ClientLog.html", 0, 10240, 10);

    asio::io_context ioContext;
    tcp::endpoint    endpoint(address::from_string("127.0.0.1"), 10005);

    Client client(ioContext, endpoint);
    client.Start();

    ioContext.run();

    return 0;
}
