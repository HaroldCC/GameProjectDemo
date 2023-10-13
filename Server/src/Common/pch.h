#pragma once

#include <vector>
#include <string>
#include <memory.h>
#include <string_view>
#include <mutex>
#include <format>
#include <type_traits>
#include <queue>

#include "include/Platform.h"
#include "asio.hpp"
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/details/null_mutex.h"
