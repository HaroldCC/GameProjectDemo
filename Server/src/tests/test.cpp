﻿#include <iostream>
#include <string>
#include <string_view>
#include <source_location>
#include "spdlog/spdlog.h"
#include "toml++/toml.h"
#include "asio.hpp"
#include "Common/include/log.hpp"
#include "Common/include/util.hpp"
#include "boost/uuid/uuid.hpp"
#include "boost/uuid/uuid_io.hpp"
#include "boost/uuid/uuid_generators.hpp"
#include "boost/beast.hpp"
#include "mysql.h"
#include "Common/threading/ProducerConsumerQueue.hpp"
#include "Common/include/Assert.h"

namespace http = boost::beast::http;

// #define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest/doctest.h"

#include "test_buffer.h"
#include "Common/include/performance.hpp"

using std::cout;
using std::endl;

using namespace std::literals;

void TestLog()
{
    for (int i = 0; i < 1; ++i)
    {
        Log::Error("==={}===", i);
        // Log::trace("current level {{{}}}", spdlog::get_level());
        // Log::debug("changed level {}", spdlog::get_level());

        Log::Info("hello {}", "demo");
        Log::Warn("this is an error{}", 101);
        Log::Error("this is a debug log {}", 202);
        Log::Critical("this is a trace log {}", 303);

        Log::Error("this is {}, this is a {}, this is {}", "hello", 1, "world");
    }

    // auto i = asio::buffer("1234");
}

class HttpRequestParser
{
public:
    HttpRequestParser() = default;

    void Parse(const std::string &httpRequest)
    {
        std::istringstream iss(httpRequest);

        // 解析请求行
        ParseRequestLine(iss);

        // 解析头部
        ParseHeaders(iss);

        // 解析请求体
        ParseBody(iss);
    }

    [[nodiscard]] std::string GetMethod() const
    {
        return _method;
    }
    [[nodiscard]] std::string GetPath() const
    {
        return path_;
    }
    [[nodiscard]] std::string GetVersion() const
    {
        return _version;
    }
    [[nodiscard]] const std::map<std::string, std::string> &GetHeaders() const
    {
        return _headers;
    }
    [[nodiscard]] std::string getBody() const
    {
        return _body;
    }

private:
    std::string                        _method;
    std::string                        path_;
    std::string                        _version;
    std::map<std::string, std::string> _headers;
    std::string                        _body;

    void ParseRequestLine(std::istringstream &iss)
    {
        std::getline(iss, _method, ' ');
        std::getline(iss, path_, ' ');
        std::getline(iss, _version, '\r');
        iss.ignore(); // 忽略掉换行符
    }

    void ParseHeaders(std::istringstream &iss)
    {
        std::string headerLine;
        while (std::getline(iss, headerLine, '\r'))
        {
            iss.ignore(); // 忽略掉换行符
            if (headerLine.empty())
            {
                break;
            }

            std::size_t colonPos = headerLine.find(':');
            if (colonPos != std::string::npos)
            {
                std::string headerName  = headerLine.substr(0, colonPos);
                std::string headerValue = headerLine.substr(colonPos + 2); // Skip the space after colon
                _headers[headerName]    = headerValue;
            }
        }
    }

    void ParseBody(std::istringstream &iss)
    {
        std::getline(iss, _body, '\0');
    }
};

void TestBoostBeastHttpParser()
{
    Util::Timer timer;
    // for (int i = 1; i < 100; ++i)
    // {
    std::string httpRequest = "GET /index.html HTTP/1.1\r\n"
                              "Host: www.example.com\r\n"
                              "User-Agent: Mozilla/5.0\r\n"
                              "Accept-Language: en-US,en;q=0.9\r\n"
                              "\r\n";

    // 创建Boost.Beast的缓冲区和请求对象
    boost::beast::flat_buffer buffer;

    // 将请求字符串解析到请求对象中
    http::request_parser<http::empty_body> parser;
    // parser.eager(true);
    boost::beast::error_code ec;
    parser.put(boost::asio::buffer(httpRequest), ec);
    // parser.parse(request, buffer, ec);

    // 检查解析是否成功
    if (ec)
    {
        std::cout << "解析HTTP请求失败：" << ec.message() << std::endl;
        return;
    }

    http::request<http::empty_body> request = parser.release();

    // 输出解析结果
    std::cout << "Method: " << request.method_string() << std::endl;
    std::cout << "Path: " << request.target() << std::endl;
    std::cout << "Version: " << request.version() << std::endl;
    std::cout << "Headers:" << std::endl;
    for (const auto &header : request)
    {
        std::cout << header.name_string() << ": " << header.value() << std::endl;
    }
    // }
}

void TestHttpParser()
{
    Util::Timer timer;
    for (int i = 1; i < 100; ++i)
    {
        std::string       httpRequest = "GET /index.html HTTP/1.1\r\n"
                                        "Host: www.example.com\r\n"
                                        "User-Agent: Mozilla/5.0\r\n"
                                        "Accept-Language: en-US,en;q=0.9\r\n"
                                        "\r\n";
        HttpRequestParser parser;
        parser.Parse(httpRequest);
        std::cout << "Method: " << parser.GetMethod() << std::endl;
        std::cout << "Path: " << parser.GetPath() << std::endl;
        std::cout << "Version: " << parser.GetVersion() << std::endl;
        std::cout << "Headers:" << std::endl;
        for (const auto &header : parser.GetHeaders())
        {
            std::cout << header.first << ": " << header.second << std::endl;
        }
    }
}

void testMysql()
{
    mysql_library_init(0, nullptr, nullptr);
    MYSQL *mysql = mysql_init(nullptr);
    mysql        = mysql_real_connect(mysql, "127.0.0.1", "root", "cr11234", "test", 3306, nullptr, 0);
    if (nullptr == mysql)
    {
        Log::Error("连接数据库失败：{}", mysql_error(mysql));
        return;
    }

    auto i = mysql_query(mysql, "select name, sex, age, weight from user");
    Log::Debug("query:{}", i);
    MYSQL_RES *pRes = mysql_store_result(mysql);

    MYSQL_ROW      pRow    = mysql_fetch_row(pRes);
    unsigned long *pLength = mysql_fetch_lengths(pRes);

    auto fieldCount = mysql_field_count(mysql);
    for (uint32_t i = 0; i < fieldCount; ++i)
    {
        Log::Debug("row:{}, length:{}", pRow[i], pLength[i]);
    }

    Log::Debug("---------------------------");
    MYSQL_STMT *pStmt = mysql_stmt_init(mysql);
    if (mysql_stmt_prepare(pStmt, "select * form user", 1024))
    {
        Log::Error("prepared statement error:{}", mysql_error(mysql));
        return;
    }
    // pRes = mysql_stmt_
}

enum class EType : uint32_t
{
    one,
    two,
    three,
};

int main(int argc, char **argv)
{
    // 读取日志配置
    // toml::parse_result config = toml::parse_file("conf/LogConfig.toml");
    // if (config.empty())
    // {
    //     return 0;
    // }

    // auto             logConfig   = config["Server"]["log"];
    // std::string_view logFileName = logConfig["name"].value_or(""sv);
    // size_t           level       = logConfig["level"].value_or(0);
    // size_t           maxFileSize = logConfig["maxfilesize"].value_or(0);
    // size_t           maxFiles    = logConfig["maxfiles"].value_or(0);
    // std::string_view pattern     = logConfig["pattern"].value_or("");

    Log::CLogger::GetLogger().InitLogger("log/log.html", 0, 10240, 10);

    TestLog();

    //  引入单侧
    doctest::Context testContext;
    testContext.applyCommandLine(argc, argv);

    int res = testContext.run();

    if (testContext.shouldExit())
    {
        return res;
    }

    // 测试
    uint8_t              header = 1;
    std::vector<uint8_t> buff{01, 22, 3, 4, 5};
    std::vector<uint8_t> packet;
    packet.reserve(sizeof(header) + buff.size());
    packet.push_back(header);
    packet.insert(packet.cend(), buff.begin(), buff.end());
    Log::Error("packet size:{}", packet.size());

    for (auto &&i : packet)
    {
        Log::Error("{}\n", i);
    }

    std::string_view content("This is a string_view !");
    // std::vector<std::string_view> subStrings = Util::Split(content, " ");
    // for (auto &&item : subStrings)
    // {
    //     Log::debug("item:{}", item);
    // }

    Log::Debug("---------------------http test---------------------------");
    // TestBoostBeastHttpParser();
    // TestHttpParser();
    // boost::beast::http::response<boost::beast::http::string_body> response;
    // response.result(boost::beast::http::status::ok);
    // response.body() = "Hello Http World!";
    // response.set("Content-Type", "text/plain;charset=utf-8");
    // response.set(boost::beast::http::field::content_length, "122");
    // response.prepare_payload();

    // boost::beast::http::response_serializer<boost::beast::http::string_body> responseSerializer(response);
    // std::ostringstream                                                       oss;
    // oss << response;
    // std::string_view responseContent = oss.view();
    // boost::beast::error_code                                             errcode;
    // responseParser.put(boost::asio::buffer(responseContent), errcode);
    // if (errcode)
    // {
    //     Log::error("parser error:{}", errcode.message());
    // }

    // responseContent = responseParser.get();
    // std::cout << responseSerializer.get() << std::endl;

    // Log::debug("{}", responseContent);

    auto testMapJoinKey = []()
    {
        Util::Timer                               timer;
        std::unordered_map<std::string_view, int> map;
        for (int i = 0; i < 10000; ++i)
        {
            std::string str1   = std::to_string(i + 2);
            std::string strKey = str1 + std::to_string(i);
            map[strKey]        = i;
        }
        Log::Debug("tesMapJoinKey map.size() = {}, {}, {}, {}", map.size(), timer.ElapsedNanosec(), timer.ElapsedMillisec(), timer.ElapsedSec());
    };

    auto testMapMap = []()
    {
        Util::Timer                                                                     timer;
        std::unordered_map<std::string_view, std::unordered_map<std::string_view, int>> map;
        for (int i = 0; i < 10000; ++i)
        {
            auto &&con                    = map[std::to_string(i + 2).data()];
            con[std::to_string(i).data()] = i;
        }

        Log::Debug("testMapMap map.size() = {}, {}, {}, {}", map.size(), timer.ElapsedNanosec(), timer.ElapsedMillisec(), timer.ElapsedSec());
    };

    testMapJoinKey();
    testMapMap();

    Log::Debug("-----------------------test base64-------------");

    // testMysql();

    Util::Timer timer;
    std::this_thread::sleep_for(2s);
    Log::Debug("micro:{}, milisec:{}, sec:{}", timer.ElapsedMicrosec(), timer.ElapsedMillisec(), timer.ElapsedSec());

    Log::Debug("---------------------------test ProducerConsumerQueue-------------------");
    ProducerConsumerQueue<int *> queueIntPointer;
    // std::thread                  tProducer([&]()
    //                       {
    //     for (int i = 0; i < 10; ++i)
    //{
    //     queueIntPointer.Push(&i);
    // }
    //     });

    for (int i = 0; i < 10; ++i)
    {
        int *tmp = new int(i);
        Log::Debug("push {} {:p})", *tmp, (void *)tmp);
        // std::cout << std::format("push {}:{:p}", i, (void *)&i);
        queueIntPointer.Push(tmp);
    }

    std::thread tConsumer([&]()
                          {
                            while(!queueIntPointer.Empty())
                                {
                            auto intPointer = queueIntPointer.WaitAndPop() ;
                            if (nullptr != intPointer)
                            {
                                // std::cout << *intPointer << std::endl;
                                Log::Debug("pop:{}", *intPointer);
                            } } });

    ProducerConsumerQueue<int> queueInt;
    // std::thread                tIntProducer([&]()
    //                          {
    //                           for (int i = 0; i < 13; ++i)
    //                           {
    //                               queueInt.Push(i);
    //                           }
    //     });

    for (int i = 0; i < 13; ++i)
    {
        queueInt.Push(i);
    }

    std::thread tIntCustomer([&]()
                             {
            while(!queueInt.Empty())
        {
                auto i = queueInt.WaitAndPop();
                                 Log::Debug("i:{}", i); } });

    // tProducer.join();
    tConsumer.join();
    // tIntProducer.join();
    tIntCustomer.join();

    Log::Debug("-------------------------------unique_ptr---------------------");
    struct A
    {
        A()
        {
            Log::Debug("A::A");
        }

        A &operator=(A &&)
        {
            Log::Debug("A::OpMtor");
        }
        A(const A &)
        {
            Log::Debug("A::Ctor");
        }

        A(A &&)
        {
            Log::Debug("A::Mtor");
        }

        A &operator=(const A &)
        {
            Log::Debug("A::opCtor");
        }

        virtual ~A()
        {
            Log::Debug("A::~A");
        }
    };

    struct B : public A
    {
        B()
        {
            Log::Debug("B::B");
        }

        B(const B &)
        {
            Log::Debug("B::Ctor");
        }

        B(B &&)
        {
            Log::Debug("B::Mtor");
        }

        B &operator=(const B &)
        {
            Log::Debug("B::OpCtor");
        }

        B &operator=(B &&)
        {
            Log::Debug("B::OpMtor");
        }

        virtual ~B()
        {
            Log::Debug("B::~B");
        }
    };

    A *pB = new B;
    // B               pTmpB = *dynamic_cast<B *>(pB);
    std::queue<A *> queueA;
    queueA.push(pB);
    std::unique_ptr<A> pSB(queueA.front());
    queueA.pop();

    Log::Debug("--------------------------------测试enum class 自动转换------------------------");
    // testMysql();
    Assert(pB == nullptr);
    Assert(pB == nullptr, "pB不为空指针");
    Assert(false, std::format("the answer is {}", 42));

    return 0;
}
