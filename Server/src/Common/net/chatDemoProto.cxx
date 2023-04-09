#include <iostream>
#include <deque>
#include <thread>
#include <asio.hpp>
#include <memory>
#include <unordered_set>
#include <google/protobuf/message.h>

using asio::ip::tcp;

class ProtoMessage
{
public:
    enum
    {
        max_length = 1024
    };

    ProtoMessage()
        : length_(0)
    {
        data_ = std::unique_ptr<char[]>(new char[max_length]);
    }

    ProtoMessage(google::protobuf::Message &message)
        : ProtoMessage()
    {
        set_message(message);
    }

    const char *data() const
    {
        return data_.get();
    }

    char *data()
    {
        return data_.get();
    }

    std::size_t length() const
    {
        return length_;
    }

    template <typename MessageType>
    bool decode(MessageType &message)
    {
        if (message.ParseFromArray(data_.get(), length_))
        {
            return true;
        }
        return false;
    }

    template <typename MessageType>
    void set_message(MessageType &message)
    {
        length_ = message.ByteSizeLong();
        if (length_ > max_length)
        {
            length_ = 0;
            return;
        }
        message.SerializeToArray(data_.get(), length_);
    }

private:
    std::unique_ptr<char[]> data_;
    std::size_t             length_;
};

class ChatRoomParticipant
{
public:
    virtual ~ChatRoomParticipant()                = default;
    virtual void deliver(const ProtoMessage &msg) = 0;
};

using ChatRoomParticipantPtr = std::shared_ptr<ChatRoomParticipant>;

class ChatRoom
{
public:
    void join(ChatRoomParticipantPtr participant)
    {
        participants_.insert(participant);
        for (auto &msg : recent_msgs_)
            participant->deliver(msg);
    }

    void leave(ChatRoomParticipantPtr participant)
    {
        participants_.erase(participant);
    }

    void deliver(const ProtoMessage &msg)
    {
        recent_msgs_.push_back(msg);
        while (recent_msgs_.size() > max_recent_msgs)
            recent_msgs_.pop_front();
        for (auto &participant : participants_)
            participant->deliver(msg);
    }

private:
    std::unordered_set<ChatRoomParticipantPtr> participants_;
    std::deque<ProtoMessage>                   recent_msgs_;
    static const int                           max_recent_msgs = 100;
};

class ChatRoomSession : public ChatRoomParticipant, public std::enable_shared_from_this<ChatRoomSession>
{
public:
    ChatRoomSession(tcp::socket &&socket, ChatRoom &room)
        : socket_(std::move(socket)), room_(room)
    {
    }

    void start()
    {
        room_.join(shared_from_this());
        do_read_header();
    }

    void deliver(const ProtoMessage &msg) override
    {
        bool write_in_progress = !write_msgs_.empty();
        write_msgs_.push_back(msg);
        if (!write_in_progress)
            do_write();
    }

private:
    void do_read_header()
    {
        auto self(shared_from_this());
        asio::async_read(socket_, asio::buffer(read_msg_.data(), 2),
                         [this, self](std::error_code ec, std::size_t /*length*/)
                         {
                             if (!ec)
                             {
                                 uint16_t len = *((uint16_t *)(read_msg_.data()));
                                 read_msg_.set_length(len);
                                 do_read_body();
                             }
                             else
                             {
                                 room_.leave(shared_from_this());
                             }
                         });
    }

    void do_read_body()
    {
        auto self(shared_from_this());
        asio::async_read(socket_, asio::buffer(read_msg_.body(), read_msg_.length()),
                         [this, self](std::error_code ec, std::size_t /*length*/)
                         {
                             if (!ec)
                             {
                                 google::protobuf::Message *message = read_msg_.get_message();
                                 if (message)
                                 {
                                     room_.deliver(read_msg_);
                                     delete message;
                                 }
                                 do_read_header();
                             }
                             else
                             {
                                 room_.leave(shared_from_this());
                             }
                         });
    }

    void do_write()
    {
        auto self(shared_from_this());
        asio::async_write(socket_, asio::buffer(write_msgs_.front().data(), write_msgs_.front().length()),
                          [this, self](std::error_code ec, std::size_t /*length*/)
                          {
                              if (!ec)
                              {
                                  write_msgs_.pop_front();
                                  if (!write_msgs_.empty())
                                      do_write();
                              }
                              else
                              {
                                  room_.leave(shared_from_this());
                              }
                          });
    }

    tcp::socket              socket_;
    ChatRoom                &room_;
    ProtoMessage             read_msg_;
    std::deque<ProtoMessage> write_msgs_;
};

class ChatRoomServer
{
public:
    ChatRoomServer(asio::io_service &io_service, const tcp::endpoint &endpoint)
        : io_service_(io_service), acceptor_(io_service, endpoint)
    {
        do_accept();
    }

private:
    void do_accept()
    {
        acceptor_.async_accept(
            [this](std::error_code ec, tcp::socket socket)
            {
                if (!ec)
                {
                    std::make_shared<ChatRoomSession>(std::move(socket), room_)->start();
                }
                do_accept();
            });
    }

    asio::io_service &io_service_;
    tcp::acceptor     acceptor_;
    ChatRoom          room_;
};

int main()
{
    try
    {
        asio::io_service io_service;
        ChatRoomServer   server(io_service, tcp::endpoint(tcp::v4(), 8080));
        io_service.run();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}

#include <asio.hpp>
#include <iostream>
#include <memory>
#include <vector>
#include "game.pb.h"

using asio::ip::tcp;

//定义会话类，负责处理数据的读取和写入
class Session : public std::enable_shared_from_this<Session> {
 public:
  Session(tcp::socket socket) : socket_(std::move(socket)) {}

  void start() {
    readHeader();
  }

 private:
  void readHeader() {
    auto self(shared_from_this());
    asio::async_read(socket_, asio::buffer(&header_, sizeof(header_)),
                     [this, self](std::error_code ec, std::size_t length) {
                       if (!ec) {
                         readBody();
                       }
                     });
  }

  void readBody() {
    auto self(shared_from_this());
    body_.resize(header_.length);
    asio::async_read(socket_, asio::buffer(body_.data(), body_.size()),
                     [this, self](std::error_code ec, std::size_t length) {
                       if (!ec) {
                         //反序列化消息
                         Game::Message message;
                         message.ParseFromArray(body_.data(), body_.size());
                         //处理消息
                         processMessage(message);
                         readHeader();
                       }
                     });
  }

  void write(const Game::Message& message) {
    auto self(shared_from_this());
    std::string data = message.SerializeAsString();
    std::size_t length = data.length();
    std::vector<asio::const_buffer> buffers;
    buffers.push_back(asio::buffer(&length, sizeof(length)));
    buffers.push_back(asio::buffer(data.c_str(), length));
    asio::async_write(socket_, buffers,
                      [this, self](std::error_code ec, std::size_t /*length*/) {
                        if (!ec) {
                          //写入操作成功
                        }
                      });
  }

  void processMessage(const Game::Message& message) {
    //处理消息
    //...
    //回复客户端
    write(message);
  }

  tcp::socket socket_;
  Game::Header header_;
  std::vector<char> body_;
};

//定义Acceptor类，负责监听client的连接请求并创建新的会话对象
class Acceptor : public std::enable_shared_from_this<Acceptor> {
 public:
  Acceptor(asio::io_service& io_service, short port)
      : io_service_(io_service), acceptor_(io_service, tcp::endpoint(tcp::v4(), port)) {}

  void start() {
    accept();
  }

 private:
  void accept() {
    auto self(shared_from_this());
    acceptor_.async_accept([this, self](std::error_code ec, tcp::socket socket) {
      if (!ec) {
        //创建新的会话对象
        auto session = std::make_shared<Session>(std::move(socket));
        //开始会话
        session->start();
      }
      accept();
    });
  }

  asio::io_service& io_service_;
  tcp::acceptor acceptor_;

};

//游戏逻辑：添加用户信息到列表中
void addUserData(std::shared_ptr<Session> session, std::vector<std::shared_ptr<Session>>& userList) {
  userList.push_back(session);
  std::cout << "New user connected. User count: " << userList.size() << std::endl;
}

//游戏逻辑：从列表中删除用户信息
void removeUserData(std::shared_ptr<Session> session, std::vector<std::shared_ptr<Session>>& userList) {
  auto it = std::find(userList.begin(), userList.end(), session);
  if (it != userList.end()) {
    userList.erase(it);
    std::cout << "User disconnected. User count: " << userList.size() << std::endl;
  }
}

//游戏逻辑：处理收到的数据
void processGameData(std::shared_ptr<Session> session, const Game::Message& message, std::vector<std::shared_ptr<Session>>& userList) {
  //...
}

int main(int argc, char** argv) {
  try {
    if (argc != 2) {
      std::cerr << "Usage: GameServer <port>\n";
      return 1;
    }

    asio::io_service io_service;

    //创建Acceptor对象
    auto acceptor = std::make_shared<Acceptor>(io_service, std::atoi(argv[1]));
    //开始监听client连接请求
    acceptor->start();

    //存储所有的Session对象
    std::vector<std::shared_ptr<Session>> userList;

    //主循环
    while (true) {
      //处理游戏逻辑
      //...
    }

  } catch (std::exception& e) {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}