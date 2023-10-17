/*************************************************************************
> File Name       : HttpRouter.cpp
> Brief           : Http路由
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月20日  11时53分00秒
************************************************************************/
#include "HttpRouter.h"

#include "Common/include/Log.hpp"
#include "boost/beast.hpp"

namespace Http
{
    std::optional<HttpResponse> HttpRouter::Handle(const HttpRequest &req)
    {
        Verb mtd = Verb::unknown;
        try
        {
            mtd = static_cast<Verb>(boost::beast::http::string_to_verb(req.GetMethod()));
        }
        catch (const std::exception &e)
        {
            Log::Error("添加路由失败：{} 路由信息：[{} {}]", e.what(), req.GetMethod(), req.GetPath());
            return std::nullopt;
        }

        if (auto iter = _tree.find(mtd); iter != _tree.end())
        {
            if (auto node = iter->second.FindNode(req.GetPath()); node.has_value())
            {
                return node.value()(req);
            }
        }

        return std::nullopt;
    }

    void HttpRouter::AddRouter(Verb method, std::string_view path, HttpHandlerFunc handler)
    {
        _tree[method].AddNode(path, std::move(handler));
    }

    void HttpRouter::AddRouter(std::string_view method, std::string_view path, HttpHandlerFunc handler)
    {
        Verb mtd = Verb::unknown;
        try
        {
            mtd = static_cast<Verb>(boost::beast::http::string_to_verb(method));
        }
        catch (const std::exception &e)
        {
            Log::Error("添加路由失败：{} 路由信息：[{} {}]", e.what(), method, path);
            return;
        }

        AddRouter(mtd, path, std::move(handler));
    }
} // namespace Http