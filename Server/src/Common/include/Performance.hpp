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

public:
    Timer()
    {
        Reset();
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
    std::chrono::time_point<std::chrono::high_resolution_clock>
        _startTimePoint;
};