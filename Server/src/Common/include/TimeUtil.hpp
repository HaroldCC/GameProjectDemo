/*************************************************************************
> File Name       : TimeUtil.hpp
> Brief           : 时间工具集
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年10月25日  16时34分47秒
************************************************************************/
#pragma once
#include <string_view>
#include <format>
#include <chrono>

namespace Util::TimeUtil
{
    inline std::string GetGmtTimeStr()
    {
        return std::format("{:%a, %d %b %Y %H:%M:%OS GMT}", std::chrono::system_clock::now());
    }
} // namespace Util::TimeUtil