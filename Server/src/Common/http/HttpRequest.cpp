/*************************************************************************
> File Name       : HttpRequest.cpp
> Brief           : http请求
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月16日  20时48分08秒
************************************************************************/
#include "pch.h"
#include <ranges>
#include "HttpRequest.h"
#include "include/util.hpp"

namespace Http
{
    WebStatus HttpRequest::Parse(std::string_view content)
    {
        if (content.empty())
        {
            return WebStatus::Error;
        }

        auto splitted = content | std::views::split(' ') | std::views::transform([](auto word)
                                                                                 { return std::string_view(word.begin(), word.end()); });
#if defined(__cpp_lib_ranges_to_container)
        std::vector<std::string_view> subStrings = content | std::ranges::views::lazy_split(' ') | std::ranges::to<std::vector<std::string_view>>;
#else
        auto &&subStrings = Util::Split(content, " ");
#endif

        if (subStrings.empty())
        {
            return WebStatus::Error;
        }

        // 请求方法
        if (subStrings[0].empty())
        {
            return WebStatus::NoImplement;
        }
        _headers["method"] = subStrings[0];

        return WebStatus::Ok;
    }
} // namespace Http