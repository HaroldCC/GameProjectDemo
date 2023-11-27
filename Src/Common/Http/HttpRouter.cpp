/*************************************************************************
> File Name       : HttpRouter.cpp
> Brief           : Http路由
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月20日  11时53分00秒
************************************************************************/
#include "HttpRouter.h"

#include "Common/Util/Log.hpp"
#include "HttpUtil.hpp"

namespace Http
{
    void HttpRouter::Route(const HttpRequest &req, HttpResponse &resp)
    {
        std::string methodWholeName = std::format("{} {}", req.GetMethod(), req.GetPath());

        if (auto iter = _httpHandlers.find(methodWholeName); iter != _httpHandlers.end())
        {
            try
            {
                iter->second(req, resp);
            }
            catch (const std::exception &e)
            {
                Log::Critical("Http方法抛出异常, reason:{}", e.what());
                resp.SetStatusCode(Status::ServiceUnavailable);
            }
            catch (...)
            {
                Log::Critical("Http抛出未知异常!!!");
                resp.SetStatusCode(Status::ServiceUnavailable);
            }
        }
        else
        {
            resp.SetStatusCode(Status::NotFound);
        }
    }

    void HttpRouter::AddHttpHandler(HttpMethod method, std::string_view path, HttpHandlerFunc handler)
    {
        std::string_view methodName = HttpMethodToString(method);
        std::string      wholeStr   = std::format("{} {}", methodName, path);

        auto [it, ok] = _pathKeys.emplace(std::move(wholeStr));
        if (!ok)
        {
            Log::Critical("http {} 已经注册");
            return;
        }

        _httpHandlers.emplace(*it, std::move(handler));
    }
} // namespace Http