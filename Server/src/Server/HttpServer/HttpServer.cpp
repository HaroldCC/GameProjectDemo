/*************************************************************************
> File Name       : HttpServer.cpp
> Brief           : Http服务器
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月21日  15时53分38秒
************************************************************************/
#include "HttpServer.h"
#include "Common/include/Log.hpp"
#include "Common/include/Assert.h"
#include "Common/database/LoginDatabase.h"

using boost::beast::http::field;
using boost::beast::http::status;
using boost::beast::http::verb;

void HttpServer::DoAccept()
{
    _accepter.async_accept(
        [this](const std::error_code &errcode, asio::ip::tcp::socket socket)
        {
            if (errcode)
            {
                Log::Error("接受连接失败：{}", errcode.message());
                return;
            }

            auto pSession = std::make_shared<Http::HttpSession>(std::move(socket));
            InitHttpRouter(pSession);
            pSession->StartSession();

            DoAccept();
        });
}

void HttpServer::InitHttpRouter(const std::shared_ptr<Http::HttpSession> &pSession)
{
    Assert(nullptr != pSession);

    pSession->AddRouter(verb::get, "/",
                        [](const Http::HttpRequest &request) -> std::optional<Http::HttpResponse>
                        {
                            Http::HttpResponse resp;
                            resp.SetStatusCode(status::ok);
                            resp.SetHeader(field::content_type, "text/html;charset=utf-8bom");
                            std::string_view body = "Hello 你好!";
                            resp.SetContent(body);
                            return resp;
                        });
    pSession->AddRouter(verb::get, "/userInfo", std::bind(&HttpServer::GetUserInfo, this, std::placeholders::_1));
    // pSession->AddRouter(verb::get, "/userInfo", [this](const Http::HttpRequest &request)
    //                     { return GetUserInfo(request); });
}

std::optional<Http::HttpResponse> HttpServer::GetUserInfo(const Http::HttpRequest &request)
{
    Http::HttpResponse              resp;
    LoginDatabasePreparedStatement *pStmt = g_LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_BY_EMAIL);
    resp.SetStatusCode(status::ok);
    return std::nullopt;
}