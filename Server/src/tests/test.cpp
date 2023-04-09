#include <iostream>
#include <string>
#include "spdlog/spdlog.h"
#include "toml++/toml.h"
#include "asio.hpp"
#include "Common/log.hpp"
#include "Common/net/session.hpp"

// #define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN

#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest/doctest.h"

#include "test_buffer.h"

using std::cout;
using std::endl;

using namespace std::literals;

void testLog()
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
    toml::parse_result config = toml::parse_file("conf/LogConfig.toml");
    if (config.empty())
    {
        return 0;
    }

    auto             logConfig   = config["Server"]["log"];
    std::string_view logFileName = logConfig["name"].value_or(""sv);
    size_t           level       = logConfig["level"].value_or(0);
    size_t           maxFileSize = logConfig["maxfilesize"].value_or(0);
    size_t           maxFiles    = logConfig["maxfiles"].value_or(0);
    std::string_view pattern     = logConfig["pattern"].value_or("");

    Log::CLogger::GetLogger().InitLogger(logFileName, level, maxFileSize, maxFiles, pattern);

    testLog();

    //  引入单侧
    doctest::Context testContext;
    testContext.applyCommandLine(argc, argv);

    int res = testContext.run();

    if (testContext.shouldExit())
    {
        return res;
    }

    return 0;
}