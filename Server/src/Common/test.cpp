
#include <iostream>
#include <string>
#include "spdlog/spdlog.h"
#include "toml++/toml.h"

using std::cout;
using std::endl;

import common;

using namespace std::literals;

void testLog()
{
    for (int i = 0; i < 1; ++i)
    {
        logger::error("==={}===", i);
        logger::info("current level {{{}}}", spdlog::get_level());
        logger::info("changed level {}", spdlog::get_level());

        logger::info("hello {}", "demo");
        logger::error("this is an error{}", 101);
        logger::debug("this is a debug log {}", 202);
        logger::trace("this is a trace log {}", 303);
    }
}

void testConfig()
{
    auto             config                 = toml::parse_file("test.toml");
    std::string_view library_name           = config["library"]["name"].value_or(""sv);
    std::string_view library_author         = config["library"]["authors"][0].value_or(""sv);
    int64_t          depends_on_cpp_version = config["dependencies"]["cpp"].value_or(0);
}

int main(int argc, char **argv)
{
    logger::CLogger::GetLogger();

    testLog();
    testConfig();

    return 0;
}