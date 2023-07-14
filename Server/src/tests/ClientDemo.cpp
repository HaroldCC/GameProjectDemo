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

    void ReadMessage()
    {
        asio::async_read(_socket, asio::buffer(&_header, sizeof(_header)),
                         [this](const std::error_code &errcode, size_t len)
                         {
                             if (errcode)
                             {
                                 Log::error("读取消息头出错：{}", errcode.message());
                                 return;
                             }

                             _header = asio::detail::socket_ops::network_to_host_long(_header);
                             _buffer.resize(_header);

                             // 读取消息体
                             asio::async_read(_socket, asio::buffer(_buffer),
                                              [this](const std::error_code &errcode, size_t len)
                                              {
                                                  if (errcode)
                                                  {
                                                      Log::error("读取消息体出错：{}", errcode.message());
                                                      return;
                                                  }

                                                  MessageDef::Message message;
                                                  if (message.ParseFromArray(_buffer.data(), (int)_buffer.size()))
                                                  {
                                                      Log::debug("接收到消息【header:{}, content:{}】", message.header(), message.content());
                                                  }
                                              });
                         });
    }

    void SendMessage()
    {
        MessageDef::Message message;
        std::string         strTest = "Hello Server";
        message.set_header(5);
        message.set_content(strTest);

        std::vector<uint8_t> content(message.ByteSizeLong());
        if (message.SerializeToArray(content.data(), (int)content.size()))
        {
            uint32_t header = asio::detail::socket_ops::host_to_network_long((int)content.size());
            // std::vector<uint8_t> buffSend;
            // buffSend.reserve(content.size() + sizeof(header));
            // buffSend.insert(buffSend.end(), (uint8_t *)&header, (uint8_t *)&header + sizeof(header));
            // buffSend.insert(buffSend.cend(), content.begin(), content.end());
            net::MessageBuffer buffSend(content.size() + sizeof(header));
            buffSend.Write(&header, sizeof(header));
            buffSend.Write(content.data(), content.size());
            asio::async_write(_socket, asio::buffer(buffSend.GetReadPointer(), buffSend.ReadableBytes()),
                              [this](const std::error_code &errcode, std::size_t len)
                              {
                                  if (errcode)
                                  {
                                      Log::error("发送消息失败：{}", errcode.message());
                                      return;
                                  }

                                  ReadMessage();
                              });
        }
    }

private:
    asio::io_context    &_ioContext;
    tcp::socket          _socket;
    tcp::endpoint        _endPoint;
    uint32_t             _header{};
    std::vector<uint8_t> _buffer;
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

// #include <iostream>
// #include <asio.hpp>

// int main()
// {
//     try
//     {
//         asio::io_context ioContext;

//         // 连接服务器
//         asio::ip::tcp::socket socket(ioContext);
//         socket.connect(asio::ip::tcp::endpoint(asio::ip::address::from_string("127.0.0.1"), 8888));

//         // 构建要发送的消息
//         MessageDef::Message message;
//         message.set_headcode(1);               // 设置消息头
//         message.set_content("Hello, server!"); // 设置消息体

//         // 序列化消息
//         std::string serializedMessage;
//         if (message.SerializeToString(&serializedMessage))
//         {
//             int32_t header = static_cast<int32_t>(serializedMessage.size());

//             // 发送消息给服务器
//             asio::write(socket, asio::buffer(&header, sizeof(header)));
//             asio::write(socket, asio::buffer(serializedMessage));

//             // 读取并打印服务器回复的消息头
//             int32_t responseHeader;
//             asio::read(socket, asio::buffer(&responseHeader, sizeof(responseHeader)));

//             // 读取并打印服务器回复的消息体
//             std::vector<char> responseBody(responseHeader);
//             asio::read(socket, asio::buffer(responseBody));

//             MessageDef::Message responseMessage;
//             if (responseMessage.ParseFromArray(responseBody.data(), (int)responseBody.size()))
//             {
//                 // 处理服务器回复的消息
//                 // 根据 responseMessage 中的字段执行相应的操作
//                 std::cout << "Server replied - Header: " << responseMessage.headcode() << " Content: " << responseMessage.content() << std::endl;
//             }
//             else
//             {
//                 std::cerr << "Error parsing response message." << std::endl;
//             }
//         }
//         else
//         {
//             std::cerr << "Error serializing message." << std::endl;
//         }
//     }
//     catch (std::exception &e)
//     {
//         std::cerr << "Exception: " << e.what() << std::endl;
//     }

//     return 0;
// }

// #include <iostream>
// #include <asio.hpp>
// #include <utility>

// class Client
// {
// public:
//     Client(asio::io_context &ioContext, asio::ip::tcp::endpoint endpoint)
//         : ioContext_(ioContext), socket_(ioContext), endpoint_(std::move(endpoint))
//     {
//     }

//     void start()
//     {
//         socket_.async_connect(endpoint_, [this](const asio::error_code &error)
//                               {
//             if (!error)
//             {
//                 // 连接成功后发送消息
//                 sendRequest();
//             }
//             else
//             {
//                 std::cerr << "Error connecting to server: " << error.message() << std::endl;
//             } });
//     }

// private:
//     void sendRequest()
//     {
//         // 构建要发送的消息
//         MessageDef::Message message;
//         message.set_headcode(1);               // 设置消息头
//         message.set_content("Hello, server!"); // 设置消息体

//         std::string serializedMessage;
//         if (message.SerializeToString(&serializedMessage))
//         {
//             // 组装消息头
//             uint32_t          header = htonl((int)serializedMessage.size());
//             std::vector<char> headerData(reinterpret_cast<char *>(&header), reinterpret_cast<char *>(&header) + sizeof(header));

//             // 组装消息体
//             std::vector<char> body(serializedMessage.begin(), serializedMessage.end());

//             // 组合消息头和消息体
//             std::vector<asio::const_buffer> buffers;
//             buffers.emplace_back(asio::buffer(headerData));
//             buffers.emplace_back(asio::buffer(body));

//             // 发送消息给服务器
//             asio::async_write(socket_, buffers,
//                               [this](const asio::error_code &error, std::size_t /*bytes_transferred*/)
//                               {
//                                   if (!error)
//                                   {
//                                       // 消息发送成功后等待接收服务器回复
//                                       readResponse();
//                                   }
//                                   else
//                                   {
//                                       std::cerr << "Error sending request: " << error.message() << std::endl;
//                                   }
//                               });
//         }
//         else
//         {
//             std::cerr << "Error serializing message." << std::endl;
//         }
//     }

//     void readResponse()
//     {
//         asio::async_read(socket_, asio::buffer(&responseHeader_, sizeof(responseHeader_)),
//                          [this](const asio::error_code &error, std::size_t /*bytes_transferred*/)
//                          {
//                              if (!error)
//                              {
//                                  responseHeader_ = ntohl(responseHeader_);
//                                  response_.resize(responseHeader_);

//                                  asio::async_read(socket_, asio::buffer(response_),
//                                                   [this](const asio::error_code &error, std::size_t /*bytes_transferred*/)
//                                                   {
//                                                       if (!error)
//                                                       {
//                                                           MessageDef::Message responseMessage;
//                                                           if (responseMessage.ParseFromArray(response_.data(), (int)response_.size()))
//                                                           {
//                                                               // 处理服务器回复的消息
//                                                               // 根据 responseMessage 中的字段执行相应的操作
//                                                               std::cout << "Server replied - Header: " << responseMessage.headcode() << " Content: " << responseMessage.content() << std::endl;
//                                                           }
//                                                           else
//                                                           {
//                                                               std::cerr << "Error parsing response message." << std::endl;
//                                                           }
//                                                       }
//                                                       else
//                                                       {
//                                                           std::cerr << "Error reading response: " << error.message() << std::endl;
//                                                       }
//                                                   });
//                              }
//                              else
//                              {
//                                  std::cerr << "Error reading response header: " << error.message() << std::endl;
//                              }
//                          });
//     }

// private:
//     asio::io_context       &ioContext_;
//     asio::ip::tcp::socket   socket_;
//     asio::ip::tcp::endpoint endpoint_;
//     uint32_t                responseHeader_;
//     std::vector<char>       response_;
// };

// int main()
// {
//     try
//     {
//         asio::io_context        ioContext;
//         asio::ip::tcp::endpoint endpoint(asio::ip::address::from_string("127.0.0.1"), 8888);
//         Client                  client(ioContext, endpoint);
//         client.start();
//         ioContext.run();
//     }
//     catch (std::exception &e)
//     {
//         std::cerr << "Exception: " << e.what() << std::endl;
//     }

//     return 0;
// }

// #include <iostream>
// #include <asio.hpp>

// class Client
// {
// public:
//     Client(asio::io_context &ioContext, const asio::ip::tcp::endpoint &endpoint)
//         : ioContext_(ioContext), socket_(ioContext), endpoint_(endpoint)
//     {
//     }

//     void start()
//     {
//         socket_.async_connect(endpoint_, [this](const asio::error_code &error)
//                               {
//             if (!error)
//             {
//                 // 连接成功后发送消息
//                 sendRequest();
//             }
//             else
//             {
//                 std::cerr << "Error connecting to server: " << error.message() << std::endl;
//             } });
//     }

// private:
//     void sendRequest()
//     {
//         // 构建要发送的消息
//         MessageDef::Message message;
//         message.set_header(1);                 // 设置消息头
//         message.set_content("Hello, server!"); // 设置消息体

//         std::vector<char> serializedMessage(message.ByteSizeLong());
//         message.SerializeToArray(serializedMessage.data(), serializedMessage.size());

//         // 组装消息头
//         uint32_t          header = htonl(serializedMessage.size());
//         std::vector<char> headerData(reinterpret_cast<char *>(&header), reinterpret_cast<char *>(&header) + sizeof(header));

//         // 组合消息头和消息体
//         // std::vector<asio::const_buffer> bufferSend;
//         // bufferSend.push_back(asio::buffer(headerData));
//         // bufferSend.push_back(asio::buffer(serializedMessage));
//         std::vector<uint8_t> bufferSend;
//         bufferSend.reserve(serializedMessage.size() + sizeof(header));
//         bufferSend.insert(bufferSend.end(), headerData.begin(), headerData.end());
//         bufferSend.insert(bufferSend.end(), serializedMessage.begin(), serializedMessage.end());

//         // 发送消息给服务器
//         asio::async_write(socket_, asio::buffer(bufferSend),
//                           [this](const asio::error_code &error, std::size_t /*bytes_transferred*/)
//                           {
//                               if (!error)
//                               {
//                                   // 消息发送成功后等待接收服务器回复
//                                   readResponse();
//                               }
//                               else
//                               {
//                                   std::cerr << "Error sending request: " << error.message() << std::endl;
//                               }
//                           });
//     }

//     void readResponse()
//     {
//         asio::async_read(socket_, asio::buffer(&responseHeader_, sizeof(responseHeader_)),
//                          [this](const asio::error_code &error, std::size_t /*bytes_transferred*/)
//                          {
//                              if (!error)
//                              {
//                                  responseHeader_ = ntohl(responseHeader_);
//                                  response_.resize(responseHeader_);

//                                  asio::async_read(socket_, asio::buffer(response_),
//                                                   [this](const asio::error_code &error, std::size_t /*bytes_transferred*/)
//                                                   {
//                                                       if (!error)
//                                                       {
//                                                           MessageDef::Message responseMessage;
//                                                           if (responseMessage.ParseFromArray(response_.data(), response_.size()))
//                                                           {
//                                                               // 处理服务器回复的消息
//                                                               // 根据 responseMessage 中的字段执行相应的操作
//                                                               std::cout << "Server replied - Header: " << responseMessage.header() << " Content: " << responseMessage.content() << std::endl;
//                                                           }
//                                                           else
//                                                           {
//                                                               std::cerr << "Error parsing response message." << std::endl;
//                                                           }
//                                                       }
//                                                       else
//                                                       {
//                                                           std::cerr << "Error reading response: " << error.message() << std::endl;
//                                                       }
//                                                   });
//                              }
//                              else
//                              {
//                                  std::cerr << "Error reading response header: " << error.message() << std::endl;
//                              }
//                          });
//     }

// private:
//     asio::io_context       &ioContext_;
//     asio::ip::tcp::socket   socket_;
//     asio::ip::tcp::endpoint endpoint_;
//     uint32_t                responseHeader_;
//     std::vector<char>       response_;
// };

// int main()
// {
//     try
//     {
//         asio::io_context        ioContext;
//         asio::ip::tcp::endpoint endpoint(asio::ip::address::from_string("127.0.0.1"), 8888);
//         Client                  client(ioContext, endpoint);
//         client.start();
//         ioContext.run();
//     }
//     catch (std::exception &e)
//     {
//         std::cerr << "Exception: " << e.what() << std::endl;
//     }

//     return 0;
// }
