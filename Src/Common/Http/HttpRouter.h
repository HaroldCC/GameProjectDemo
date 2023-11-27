/*************************************************************************
> File Name       : HttpRouter.h
> Brief           : Http路由功能
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月17日  20时28分46秒
************************************************************************/
#pragma once
#include <functional>
#include "HttpRequest.h"
#include "HttpResponse.h"
#include <unordered_map>
#include <set>

namespace Http
{
    using HttpHandlerFunc = std::function<void(const HttpRequest &, HttpResponse &)>;

    class HttpRouter final
    {
    public:
        void Route(const HttpRequest &req, HttpResponse &resp);

        /**
         * @brief 添加Http处理函数  eg: GET hello/    POST hello/ 作为key
         * 
         * @param method 方法
         * @param path 路径
         * @param handler 处理函数
         */
        void AddHttpHandler(HttpMethod method, std::string_view path, HttpHandlerFunc handler);

    private:
        std::unordered_map<std::string_view, HttpHandlerFunc> _httpHandlers;
        std::set<std::string>                                 _pathKeys;
    };
} // namespace Http
