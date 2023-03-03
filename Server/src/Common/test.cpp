
#include "spdlog/spdlog.h"
#include <iostream>

using std::cout;
using std::endl;

import common;

void testLog()
{
    logger::info("current level {}", spdlog::get_level());
    spdlog::set_level(spdlog::level::trace);
    logger::info("changed level {}", spdlog::get_level());

    logger::info("hello {}", "demo");
    logger::error("this is an error{}", 101);
    logger::debug("this is a debug log {}", 202);
    logger::trace("this is a trace log {}", 303);
}

int main(int argc, char **argv)
{
    testLog();

    return 0;
}