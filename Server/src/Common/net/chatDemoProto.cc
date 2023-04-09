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