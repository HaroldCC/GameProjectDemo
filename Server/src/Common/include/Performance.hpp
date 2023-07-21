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

class Timer
{
private:
    std::chrono::time_point<std::chrono::high_resolution_clock> _mStartTimePoint;

public:
    Timer()
        : _mStartTimePoint(std::chrono::high_resolution_clock::now())
    {
    }

    Timer(const Timer &)            = default;
    Timer(Timer &&)                 = delete;
    Timer &operator=(const Timer &) = default;
    Timer &operator=(Timer &&)      = delete;
    ~Timer()
    {
        Stop();
    }

    void Stop()
    {
        auto endTimePoint = std::chrono::high_resolution_clock::now();

        auto start = std::chrono::time_point_cast<std::chrono::microseconds>(_mStartTimePoint)
                         .time_since_epoch()
                         .count();
        auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimePoint)
                       .time_since_epoch()
                       .count();

        auto   duration    = end - start;
        double miliseconds = (double)duration * 0.001;

        std::cout << "**********" << duration << "us (" << miliseconds << "ms)\n";
    }
};