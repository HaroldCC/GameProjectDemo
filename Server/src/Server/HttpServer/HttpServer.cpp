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
    pSession->AddRouter(verb::get, "/sql",
                        [](const Http::HttpRequest &request) -> Http::HttpResponse
                        {
                            Http::HttpResponse resp;
                            ResultSetPtr       pResult = g_LoginDatabase.SyncQuery("select id, email, name, age from account where id = 1");
                            if (nullptr == pResult)
                            {
                                resp.SetStatusCode(status::internal_server_error);
                            }
                            else
                            {
                                Field *pFields = pResult->Fetch();
                                // resp.SetStatusCode(status::ok);
                                // resp.SetHeader(field::content_type, "text/html;charset=utf-8");
                                // resp.SetContent(std::format("id:{}, email:{}, name:{}, age:{}", ()pFields[0], pFields[1], pFields[2], pFields[3]));
                            }

                            return resp;
                        });
}

std::optional<Http::HttpResponse> HttpServer::GetUserInfo(const Http::HttpRequest &request)
{
    Http::HttpResponse resp;
    auto               pAccountInfoStmt = g_LoginDatabaseStmtMap.find(LOGIN_SEL_ACCOUNT_BY_EMAIL);
    if (pAccountInfoStmt == g_LoginDatabaseStmtMap.end())
    {
        resp.SetStatusCode(status::internal_server_error);
    }
    else
    {

        LoginDatabasePreparedStatement *pStmt = g_LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_BY_EMAIL);
        for (size_t i = 0; i < pAccountInfoStmt->second.argTypes.size(); ++i)
        {
            std::string strEmail{"123456@qq.com"};
            pStmt->SetValue((uint8_t)i, pAccountInfoStmt->second.argTypes[i], strEmail);
        }

        PreparedResultSetPtr pResult = g_LoginDatabase.SyncQuery(pStmt);
        if (nullptr == pResult)
        {
            resp.SetStatusCode(status::internal_server_error);
            return resp;
        }

        Field      *pFields = pResult->Fetch();
        uint8_t     id      = pFields[0];
        const char *strName = pFields[1];
        const char *email   = pFields[2];
        uint32_t    age     = pFields[3];
        std::string intro   = pFields[4];

        ResultSetPtr                 result = g_LoginDatabase.SyncQuery("select id, name, email, age, intro from account where email= '123456@qq.com'");
        Field                       *field  = result->Fetch();
        [[maybe_unused]] uint32_t    id2    = field[0];
        [[maybe_unused]] const char *name   = field[1];

        resp.SetHeader(field::content_type, "text/html;charset=utf-8");
        resp.SetContent(std::format("id:{}, email:{}, name:{}, age:{}, intro:{}", id, email, strName, age, intro));
        Log::Debug("id:{}, email:{}, name:{}, age:{}, intro:{}", id, email, strName, age, intro);

        resp.SetStatusCode(status::ok);
    }
    return resp;
}