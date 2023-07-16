#include <iostream>
#include <string>
#include <string_view>
#include "spdlog/spdlog.h"
#include "toml++/toml.h"
#include "asio.hpp"
#include "Common/include/log.hpp"
#include "Common/include/util.hpp"
#include "boost/beast.hpp"

namespace http = boost::beast::http;

// #define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest/doctest.h"

#include "test_buffer.h"

using std::cout;
using std::endl;

using namespace std::literals;

void TestLog()
{
    for (int i = 0; i < 1; ++i)
    {
        Log::error("==={}===", i);
        // Log::trace("current level {{{}}}", spdlog::get_level());
        // Log::debug("changed level {}", spdlog::get_level());

        Log::info("hello {}", "demo");
        Log::warn("this is an error{}", 101);
        Log::error("this is a debug log {}", 202);
        Log::critical("this is a trace log {}", 303);

        Log::error("this is {}, this is a {}, this is {}", "hello", 1, "world");
    }

    // auto i = asio::buffer("1234");
}

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
    Log::error("packet size:{}", packet.size());

    for (auto &&i : packet)
    {
        Log::error("{}\n", i);
    }

    std::string_view              content("This is a string_view !");
    std::vector<std::string_view> subStrings = Util::Split(content, " ");
    for (auto &&item : subStrings)
    {
        Log::debug("item:{}", item);
    }

    Log::debug("---------------------http test---------------------------");
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
        return 1;
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
    return 0;
}