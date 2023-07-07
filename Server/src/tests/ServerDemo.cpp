#include "net/server.h"
#include "Common/log.hpp"

int main()
{
    Log::CLogger::GetLogger().InitLogger("log/ServerLog.html", 0, 10240, 10);

    asio::io_context ioContext;

    net::Server server(ioContext, 8899);
    server.Start();

    ioContext.run();

    return 0;
}

#include <iostream>
#include <memory>
#include "Common/include/platform.h"
#include <asio.hpp>
#include "MessageDef.pb.h"

#include <iostream>
#include <memory>
#include <asio.hpp>

// class Session : public std::enable_shared_from_this<Session>
// {
// public:
//     Session(asio::ip::tcp::socket socket)
//         : socket_(std::move(socket))
//     {
//     }

//     void start()
//     {
//         readHeader();
//     }

// private:
//     void readHeader()
//     {
//         auto self(shared_from_this());

//         asio::async_read(socket_, asio::buffer(&header_, sizeof(header_)),
//                          [this, self](const asio::error_code &error, std::size_t /*bytes_transferred*/)
//                          {
//                              if (!error)
//                              {
//                                  std::cout << "header before = " << header_ << std::endl;
//                                  //  header_ = ntohl(header_);
//                                  std::cout << "header = " << header_ << std::endl;
//                                  readBody();
//                              }
//                              else
//                              {
//                                  std::cerr << "Error reading header: " << error.message() << std::endl;
//                              }
//                          });
//     }

//     void readBody()
//     {
//         auto self(shared_from_this());

//         body_.resize(header_);
//         asio::async_read(socket_, asio::buffer(body_),
//                          [this, self](const asio::error_code &error, std::size_t /*bytes_transferred*/)
//                          {
//                              if (!error)
//                              {
//                                  MessageDef::Message message;
//                                  if (message.ParseFromArray(body_.data(), (int)body_.size()))
//                                  {
//                                      handleReceivedMessage(message);
//                                  }
//                                  else
//                                  {
//                                      std::cerr << "Error parsing message." << std::endl;
//                                  }

//                                  readHeader();
//                              }
//                              else
//                              {
//                                  std::cerr << "Error reading body: " << error.message() << std::endl;
//                              }
//                          });
//     }

//     void handleReceivedMessage(const MessageDef::Message &message)
//     {
//         // 在这里处理接收到的消息
//         // 根据 message 中的字段执行相应的操作
//         // 例如，您可以执行游戏逻辑，然后生成响应消息，并通过 sendResponse() 方法发送回客户端
//         std::cout << "接受客户端消息" << message.headcode() << "," << message.content() << std::endl;

//         MessageDef::Message response;
//         response.set_headcode(2);
//         response.set_content("Hello Client");
//         sendResponse(response);
//     }

//     void sendResponse(const MessageDef::Message &message)
//     {
//         std::string serializedMessage;
//         if (message.SerializeToString(&serializedMessage))
//         {
//             uint32_t header = static_cast<uint32_t>(serializedMessage.size());

//             asio::write(socket_, asio::buffer(&header, sizeof(header)));
//             asio::write(socket_, asio::buffer(serializedMessage));
//         }
//         else
//         {
//             std::cerr << "Error serializing message." << std::endl;
//         }
//     }

// private:
//     asio::ip::tcp::socket socket_;
//     uint32_t              header_;
//     std::vector<char>     body_;
// };

// class Server
// {
// public:
//     Server(asio::io_context &ioContext, const asio::ip::tcp::endpoint &endpoint)
//         : ioContext_(ioContext), acceptor_(ioContext, endpoint)
//     {
//     }

//     void start()
//     {
//         acceptConnection();
//     }

// private:
//     void acceptConnection()
//     {
//         acceptor_.async_accept([this](const asio::error_code &error, asio::ip::tcp::socket socket)
//                                {
//             if (!error)
//             {
//                 std::cout << "Accepted new connection from: " << socket.remote_endpoint() << std::endl;

//                 auto session = std::make_shared<Session>(std::move(socket));
//                 session->start();

//                 // 在这里可以将 session 存储起来，例如放入一个容器，以便管理会话
//             }
//             else
//             {
//                 std::cerr << "Error accepting connection: " << error.message() << std::endl;
//             }

//             acceptConnection(); });
//     }

// private:
//     asio::io_context       &ioContext_;
//     asio::ip::tcp::acceptor acceptor_;
// };

// int main()
// {
//     try
//     {
//         asio::io_context        ioContext;
//         asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), 8888);
//         Server                  server(ioContext, endpoint);
//         server.start();
//         ioContext.run();
//     }
//     catch (std::exception &e)
//     {
//         std::cerr << "Exception: " << e.what() << std::endl;
//     }

//     return 0;
// }

// #include <iostream>
// #include <memory>
// #include <asio.hpp>

// class Session : public std::enable_shared_from_this<Session>
// {
// public:
//     Session(asio::ip::tcp::socket socket)
//         : socket_(std::move(socket))
//     {
//     }

//     void start()
//     {
//         readHeader();
//     }

// private:
//     void readHeader()
//     {
//         auto self(shared_from_this());

//         asio::async_read(socket_, asio::buffer(&header_, sizeof(header_)),
//                          [this, self](const asio::error_code &error, std::size_t /*bytes_transferred*/)
//                          {
//                              if (!error)
//                              {
//                                  header_ = ntohl(header_);
//                                  readBody();
//                              }
//                              else
//                              {
//                                  std::cerr << "Error reading header: " << error.message() << std::endl;
//                              }
//                          });
//     }

//     void readBody()
//     {
//         auto self(shared_from_this());

//         body_.resize(header_);
//         asio::async_read(socket_, asio::buffer(body_),
//                          [this, self](const asio::error_code &error, std::size_t /*bytes_transferred*/)
//                          {
//                              if (!error)
//                              {
//                                  MessageDef::Message message;
//                                  if (message.ParseFromArray(body_.data(), (int)body_.size()))
//                                  {
//                                      handleReceivedMessage(message);
//                                  }
//                                  else
//                                  {
//                                      std::cerr << "Error parsing message." << std::endl;
//                                  }

//                                  //  readHeader();
//                              }
//                              else
//                              {
//                                  std::cerr << "Error reading body: " << error.message() << std::endl;
//                              }
//                          });
//     }

//     void handleReceivedMessage(const MessageDef::Message &message)
//     {
//         // 在这里处理接收到的消息
//         // 根据 message 中的字段执行相应的操作
//         // 例如，您可以执行游戏逻辑，然后生成响应消息，并通过 sendResponse() 方法发送回客户端
//         std::cout << "收到客户端消息" << message.headcode() << ", " << message.content() << std::endl;

//         MessageDef::Message response;
//         response.set_headcode(211);
//         response.set_content("Hello Client");
//         sendResponse(response);
//     }

//     void sendResponse(const MessageDef::Message &message)
//     {
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

//             // 发送消息给客户端
//             asio::write(socket_, buffers);
//         }
//         else
//         {
//             std::cerr << "Error serializing message." << std::endl;
//         }
//     }

// private:
//     asio::ip::tcp::socket socket_;
//     uint32_t              header_;
//     std::vector<char>     body_;
// };

// class Server
// {
// public:
//     Server(asio::io_context &ioContext, const asio::ip::tcp::endpoint &endpoint)
//         : ioContext_(ioContext), acceptor_(ioContext, endpoint)
//     {
//     }

//     void start()
//     {
//         acceptConnection();
//     }

// private:
//     void acceptConnection()
//     {
//         acceptor_.async_accept([this](const asio::error_code &error, asio::ip::tcp::socket socket)
//                                {
//             if (!error)
//             {
//                 std::cout << "Accepted new connection from: " << socket.remote_endpoint() << std::endl;

//                 auto session = std::make_shared<Session>(std::move(socket));
//                 session->start();

//                 // 在这里可以将 session 存储起来，例如放入一个容器，以便管理会话
//             }
//             else
//             {
//                 std::cerr << "Error accepting connection: " << error.message() << std::endl;
//             }

//             acceptConnection(); });
//     }

// private:
//     asio::io_context       &ioContext_;
//     asio::ip::tcp::acceptor acceptor_;
// };

// int main()
// {
//     try
//     {
//         asio::io_context        ioContext;
//         asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), 8888);
//         Server                  server(ioContext, endpoint);
//         server.start();
//         ioContext.run();
//     }
//     catch (std::exception &e)
//     {
//         std::cerr << "Exception: " << e.what() << std::endl;
//     }

//     return 0;
// }

// #include <iostream>
// #include <memory>
// #include <asio.hpp>

// class Session : public std::enable_shared_from_this<Session>
// {
// public:
//     Session(asio::ip::tcp::socket socket)
//         : socket_(std::move(socket))
//     {
//     }

//     void start()
//     {
//         readHeader();
//     }

// private:
//     void readHeader()
//     {
//         auto self(shared_from_this());

//         asio::async_read(socket_, asio::buffer(&header_, sizeof(header_)),
//                          [this, self](const asio::error_code &error, std::size_t /*bytes_transferred*/)
//                          {
//                              if (!error)
//                              {
//                                  header_ = ntohl(header_);
//                                  readBody();
//                              }
//                              else
//                              {
//                                  std::cerr << "Error reading header: " << error.message() << std::endl;
//                              }
//                          });
//     }

//     void readBody()
//     {
//         auto self(shared_from_this());

//         body_.resize(header_);
//         asio::async_read(socket_, asio::buffer(body_),
//                          [this, self](const asio::error_code &error, std::size_t /*bytes_transferred*/)
//                          {
//                              if (!error)
//                              {
//                                  MessageDef::Message message;
//                                  if (message.ParseFromArray(body_.data(), body_.size()))
//                                  {
//                                      handleReceivedMessage(message);
//                                  }
//                                  else
//                                  {
//                                      std::cerr << "Error parsing message." << std::endl;
//                                  }

//                                  readHeader();
//                              }
//                              else
//                              {
//                                  std::cerr << "Error reading body: " << error.message() << std::endl;
//                              }
//                          });
//     }

//     void handleReceivedMessage(const MessageDef::Message &message)
//     {
//         // 在这里处理接收到的消息
//         // 根据 message 中的字段执行相应的操作
//         // 例如，您可以执行游戏逻辑，然后生成响应消息，并通过 sendResponse() 方法发送回客户端

//         std::cout << "受到客户端消息" << message.header() << ", " << message.content() << std::endl;

//         MessageDef::Message response;
//         response.set_header(211);
//         response.set_content("Hello Client");
//         sendResponse(response);
//     }

//     void sendResponse(const MessageDef::Message &message)
//     {
//         std::vector<char> serializedMessage(message.ByteSizeLong());
//         message.SerializeToArray(serializedMessage.data(), serializedMessage.size());

//         // 组装消息头
//         uint32_t          header = htonl(serializedMessage.size());
//         std::vector<char> headerData(reinterpret_cast<char *>(&header), reinterpret_cast<char *>(&header) + sizeof(header));

//         // 组合消息头和消息体
//         std::vector<asio::const_buffer> buffers;
//         buffers.push_back(asio::buffer(headerData));
//         buffers.push_back(asio::buffer(serializedMessage));

//         // 发送消息给客户端
//         asio::write(socket_, buffers);
//     }

// private:
//     asio::ip::tcp::socket socket_;
//     uint32_t              header_;
//     std::vector<char>     body_;
// };

// class Server
// {
// public:
//     Server(asio::io_context &ioContext, const asio::ip::tcp::endpoint &endpoint)
//         : ioContext_(ioContext), acceptor_(ioContext, endpoint)
//     {
//     }

//     void start()
//     {
//         acceptConnection();
//     }

// private:
//     void acceptConnection()
//     {
//         acceptor_.async_accept([this](const asio::error_code &error, asio::ip::tcp::socket socket)
//                                {
//             if (!error)
//             {
//                 std::cout << "Accepted new connection from: " << socket.remote_endpoint() << std::endl;

//                 auto session = std::make_shared<Session>(std::move(socket));
//                 session->start();

//                 // 在这里可以将 session 存储起来，例如放入一个容器，以便管理会话
//             }
//             else
//             {
//                 std::cerr << "Error accepting connection: " << error.message() << std::endl;
//             }

//             acceptConnection(); });
//     }

// private:
//     asio::io_context       &ioContext_;
//     asio::ip::tcp::acceptor acceptor_;
// };

// int main()
// {
//     try
//     {
//         asio::io_context        ioContext;
//         asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), 8888);
//         Server                  server(ioContext, endpoint);
//         server.start();
//         ioContext.run();
//     }
//     catch (std::exception &e)
//     {
//         std::cerr << "Exception: " << e.what() << std::endl;
//     }

//     return 0;
// }
