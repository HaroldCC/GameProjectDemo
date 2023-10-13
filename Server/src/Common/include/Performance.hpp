/*************************************************************************
> File Name       : performance.hpp
> Brief           : 性能检测
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月17日  10时25分20秒
************************************************************************/
#pragma once

#include <iostream>
#include <memory>
#include <chrono>
#include "Log.hpp"
#include "Util.hpp"

namespace Util
{
    class Timer
    {

    public:
        explicit Timer(std::string_view msg = "", bool logOnDestroy = false)
            : _msg(msg), _logOnDestroy(logOnDestroy)
        {
            Reset();
        }

        Timer(const Timer &)            = default;
        Timer &operator=(const Timer &) = default;
        Timer(Timer &&)                 = delete;
        Timer &operator=(Timer &&)      = delete;

        ~Timer()
        {
            if (_logOnDestroy)
            {
                Log::Debug("{} 耗时:{} ms", _msg, ElapsedMillisec());
            }
        }

        void Reset()
        {
            _startTimePoint = std::chrono::high_resolution_clock::now();
        }

        double ElapsedNanosec()
        {
            auto currentTimePoint = std::chrono::high_resolution_clock::now();
            return (double)std::chrono::duration_cast<std::chrono::nanoseconds>(currentTimePoint - _startTimePoint)
                .count();
        }

        double ElapsedMicrosec()
        {
            return ElapsedNanosec() * 0.001;
        }

        double ElapsedMillisec()
        {
            return ElapsedMicrosec() * 0.001;
        }

        double ElapsedSec()
        {
            return ElapsedMillisec() * 0.001;
        }

    private:
        std::string_view _msg;
        bool             _logOnDestroy;
        std::chrono::time_point<std::chrono::high_resolution_clock>
            _startTimePoint;
    };

} // namespace Util

#if defined(PERFORMANCE_DECT)
    #define PERFORMANCE_SCOPE_LINE(msg, line) \
        Util::Timer timer##line(msg, true)
    #define PERFORMANCE_SCOPE(msg) PERFORMANCE_SCOPE_LINE(msg, __LINE__)
#else
    #define PERFORMANCE_SCOPE(msg)
#endif
