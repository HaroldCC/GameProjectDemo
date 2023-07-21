// #include "Common/net/session.h"
// #include "testMsg.pb.h"

// using tcp = asio::ip::tcp;

// class ClientSession : public net::Session
// {
// public:
//     explicit ClientSession(tcp::socket &&socket)
//         : net::Session(std::move(socket))
//     {
//     }

// protected:
//     void ReadHandler() override
//     {
//         if (!IsAlive())
//         {
//             return;
//         }

//         net::MessageBuffer &buffer = GetReadBuffer();
//         while (buffer.ReadableBytes() > 0)
//         {
//             Proto::TestMsg testProtoMsg;
//             testProtoMsg.ParseFromArray(buffer.GetReadPointer(), (int)buffer.ReadableBytes());
//             buffer.ReadDone(buffer.ReadableBytes());

//             std::string content = std::format("receive server message:{}, id:{}", testProtoMsg.msg(), testProtoMsg.id());
//             Log::info("{}", content);

//             int id = testProtoMsg.id() + 1;
//             if (id <= 100)
//             {
//                 std::string        strMsg = std::format("hello server id:", testProtoMsg.id() + 1);
//                 net::MessageBuffer retMsgBuffer;
//                 retMsgBuffer.Write(strMsg);
//                 AsyncSendMessage(std::move(retMsgBuffer));
//             }
//         }
//     }
// };

// class Client
// {
// public:
//     Client(asio::io_context &ioContext, const)
// };

// int main()
// {
//     try
//     {
//         asio::io_context ioContext;

//         tcp::socket   s(ioContext);
//         tcp::resolver resolver(ioContext);
//         asio::connect(s, resolver.resolve("127.0.0.1", "9988"));

//         for (int i = 0; i <= 10; ++i)
//         {
//             net::MessageBuffer buffer;
//             Proto::TestMsg     msg;
//             msg.set_id(i);
//             msg.set_msg("hello server");
//             if (msg.SerializeToArray(buffer.GetWritPointer(), (int)msg.ByteSizeLong()))
//                 // buffer.Write(buffer.Data(), msg.ByteSizeLong());
//                 buffer.WriteDone(msg.ByteSizeLong());
//             asio::write(s, asio::buffer(buffer.GetReadPointer(), buffer.ReadableBytes()));
//             std::this_thread::sleep_for(std::chrono::milliseconds(1));

//             // net::MessageBuffer replayBuffer;
//             // std::error_code    error;
//             // s.async_read_some(asio::buffer(replayBuffer.Data(), replayBuffer.WritableBytes()),
//             //                   [&](const std::error_code &error, size_t len)
//             //                   {
//             //                       if (!error)
//             //                       {
//             //                           Log::error("socket read replay error :{}", error.message());
//             //                           return;
//             //                       }

//             //                       msg.ParseFromArray(replayBuffer.Data(), (int)len);
//             //                       replayBuffer.WriteInBytes(len);
//             //                       Log::info("receive server message:{}", replayBuffer.ReadAllAsString());
//             //                   });
//         }
//     }
//     catch (std::exception &e)
//     {
//         Log::error("Exception:{}", e.what());
//     }

//     system("pause");
//     try
//     {
//         if (argc != 3)
//         {
//             std::cerr << "Usage: echo_client <host> <port>\n";
//             return 1;
//         }

//         asio::io_context io_context;

//         tcp::resolver               resolver(io_context);
//         tcp::resolver::results_type endpoints = resolver.resolve(argv[1], argv[2]);

//         tcp::socket socket(io_context);
//         asio::connect(socket, endpoints);

//         std::cout << "Enter message: ";
//         std::string message;
//         while (std::getline(std::cin, message))
//         {
//             asio::write(socket, asio::buffer(message + "\n"));

//             std::array<char, 128> buf;
//             asio::error_code      error;

//             size_t len = socket.read_some(asio::buffer(buf), error);

//             if (error == asio::error::eof)
//             {
//                 std::cout << "Connection closed by peer.\n";
//                 break;
//             }
//             else if (error)
//             {
//                 throw asio::system_error(error);
//             }

//             std::cout.write(buf.data(), len);
//             std::cout << "Enter message: ";
//         }
//     }
//     catch (std::exception &e)
//     {
//         std::cerr << "Exception: " << e.what() << "\n";
//     }

//     return 0;
// }