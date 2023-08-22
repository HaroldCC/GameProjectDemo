/*************************************************************************
> File Name       : Assert.h
> Brief           : 断言
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年08月14日  19时59分36秒
************************************************************************/
#include "Platform.h"
#include "Log.hpp"
#include <filesystem>

#ifdef DEBUG
#if defined(OS_PLATFORM_WINDOWS)
#define DEBUG_BREAK() __debugbreak()
#elif defined(OS_PLATFORM_LINUX)
#include <signal.h>
#define DEBUG_BREAK() raise(SIGTRAP)
#else
#error "Platform doesn't support debug break yet!"
#endif
#define ENABLE_ASSERTS
#else
#define DEBUG_BREAK()
#endif

#define EXPAND_MACRO(x)    x
#define STRINGIFY_MACRO(x) #x

#define INTERNAL_ASSERT_IMPL(type, check, msg, ...) \
    {                                               \
        if (!(check))                               \
        {                                           \
            Log::Error(msg, __VA_ARGS__);           \
            DEBUG_BREAK();                          \
        }                                           \
    }
#define INTERNAL_ASSERT_WITH_MSG(type, check, ...) INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
#define INTERNAL_ASSERT_NO_MSG(type, check)        INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

#define INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
#define INTERNAL_ASSERT_GET_MACRO(...)                         EXPAND_MACRO(INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, INTERNAL_ASSERT_WITH_MSG, INTERNAL_ASSERT_NO_MSG))

// Currently accepts at least the condition and one additional parameter (the message) being optional
#define Assert(...) EXPAND_MACRO(INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__))