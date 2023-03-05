import common;

#include <iostream>
#include <string>
#include "spdlog/spdlog.h"
#include "toml++/toml.h"

using std::cout;
using std::endl;

using namespace std::literals;

void testLog()
{
    for (int i = 0; i < 1; ++i)
    {
        logger::error("==={}===", i);
        logger::trace("current level {{{}}}", spdlog::get_level());
        logger::debug("changed level {}", spdlog::get_level());

        logger::info("hello {}", "demo");
        logger::warn("this is an error{}", 101);
        logger::error("this is a debug log {}", 202);
        logger::cirtical("this is a trace log {}", 303);
    }
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

    logger::CLogger::GetLogger().InitLogger(logFileName, level, maxFileSize, maxFiles, pattern);

    testLog();

    return 0;
}