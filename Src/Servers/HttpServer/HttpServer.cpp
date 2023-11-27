/*************************************************************************
> File Name       : HttpServer.cpp
> Brief           : Http服务器
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月21日  15时53分38秒
************************************************************************/
#include "HttpServer.h"
#include "Common/Util/Log.hpp"
#include "Common/Util/Assert.h"
#include "Common/Database/DatabaseImpl/LoginDatabase.h"

using Http::HttpMethod;
using Http::Status;

void HttpServer::DoAccept()
{
    _accepter.async_accept([this](const std::error_code &errcode, asio::ip::tcp::socket socket) {
        if (errcode)
        {
            Log::Error("接受连接失败：{}", errcode.message());
            return;
        }

        auto pSession = std::make_shared<Http::HttpSession>(std::move(socket));
        _sessions.emplace_back(pSession);
        InitHttpRouter(pSession);
        pSession->StartSession();

        DoAccept();
    });
}

void HttpServer::InitHttpRouter(const std::shared_ptr<Http::HttpSession> &pSession)
{
    Assert(nullptr != pSession);

    pSession->AddRouter(HttpMethod::GET, "/", [](const Http::HttpRequest &request, Http::HttpResponse &resp) {
        resp.SetStatusCode(Status::Ok);
        std::string_view body = "Hello 你好!";
        resp.SetContent(body);
        return;
    });

    pSession->AddRouter(
        HttpMethod::GET,
        "/userInfo",
        std::bind(&HttpServer::GetUserInfo, this, std::placeholders::_1, std::placeholders::_2));

    pSession->AddRouter(
        HttpMethod::GET,
        "/test",
        [](const Http::HttpRequest &request, Http::HttpResponse &resp) {
            g_LoginDatabase.AsyncExecute(
                "insert into account(email, name, age) values('123@qq.com', 'tom', 20)");
            // if (nullptr == pResult)
            // {
            //     resp.SetStatusCode(Status::internal_server_error);
            // }
            // else
            // {
            //     Field *pFields = pResult->Fetch();
            //     resp.SetStatusCode(Status::ok);
            //     resp.SetHeader(Field::content_type, "text/html;charset=utf-8");
            //     resp.SetContent(std::format("id:{}, email:{}, name:{}, age:{}", pFields[0].operator uint32_t(), pFields[1].operator std::string(), pFields[2].operator std::string(), pFields[3].operator uint32_t()));
            // }
        });

    pSession->AddRouter(
        HttpMethod::GET,
        "/testAsync",
        [this](const Http::HttpRequest &request, Http::HttpResponse &resp) {
            _processor
                .AddCallback(g_LoginDatabase.AsyncQuery(
                    "select id, name, age, intro from account where email='123@qq.com'"))
                .Then([&resp](const ResultSetPtr &pResult) {
                    // Field *pFields = pResult->Fetch();
                    // int         id      = pFields[0];
                    // std::string name    = pFields[1];
                    // int         age     = pFields[2];
                    // const char *intro   = pFields[3];
                    // Log::Debug("{}", std::format("id:{}, name:{}, age:{}, intro = {}", pFields[0].operator uint32_t(), pFields[1].operator std::string(), pFields[2].operator uint32_t(), pFields[3].operator std::string()));
                    // resp.SetStatusCode(Status::ok);
                    if (nullptr == pResult)
                    {
                        // Log::Debug("{}", "查询失败");
                        resp.SetStatusCode(Status::InternalServerError);
                        resp.SetContent("结果出错！！！");
                    }
                    else
                    {
                        // Log ::Debug("{}查询失败", 1);
                        resp.SetStatusCode(Status::Ok);
                        // resp.SetContent(std::format("async id = {}, name = {}, age = {}, intro = {}", pResult->Fetch()))
                        Field *pFields = pResult->Fetch();
                        Log::Debug("{}",
                                   std::format("id:{}, name:{}, age:{}, intro = {}",
                                               pFields[0].operator uint32_t(),
                                               pFields[1].operator std::string(),
                                               pFields[2].operator uint32_t(),
                                               pFields[3].operator std::string()));
                        resp.SetContent(std::format("id:{}, name:{}, age:{}, intro = {}",
                                                    pFields[0].operator uint32_t(),
                                                    pFields[1].operator std::string(),
                                                    pFields[2].operator uint32_t(),
                                                    pFields[3].operator std::string()));
                    }
                });
            std::this_thread::sleep_for(std::chrono::seconds(3));
            Log::Debug("{}", "12343");
            return resp;
        });
}

void HttpServer::GetUserInfo(const Http::HttpRequest &request, Http::HttpResponse &resp)
{
    auto pAccountInfoStmt = g_LoginDatabaseStmtMap.find(LOGIN_SEL_ACCOUNT_BY_EMAIL);
    if (pAccountInfoStmt == g_LoginDatabaseStmtMap.end())
    {
        resp.SetStatusCode(Status::InternalServerError);
    }
    else
    {
        LoginDatabasePreparedStatement *pStmt =
            g_LoginDatabase.GetPreparedStatement(LOGIN_SEL_ACCOUNT_BY_EMAIL);
        for (size_t i = 0; i < pAccountInfoStmt->second.argTypes.size(); ++i)
        {
            std::string strEmail {"123456@qq.com"};
            pStmt->SetValue((uint8_t)i, pAccountInfoStmt->second.argTypes[i], strEmail);
        }

        PreparedResultSetPtr pResult = g_LoginDatabase.SyncQuery(pStmt);
        if (nullptr == pResult)
        {
            resp.SetStatusCode(Status::InternalServerError);
            return;
        }

        Field      *pFields = pResult->Fetch();
        uint32_t    id      = pFields[0];
        const char *strName = pFields[1];
        const char *email   = pFields[2];
        uint32_t    age     = pFields[3];
        std::string intro   = pFields[4];

        ResultSetPtr result = g_LoginDatabase.SyncQuery(
            "select id, name, email, age, intro from account where email= '123456@qq.com'");
        Field                       *field = result->Fetch();
        [[maybe_unused]] uint32_t    id2   = field[0];
        [[maybe_unused]] const char *name  = field[1];

        resp.SetContent(
            std::format("id:{}, email:{}, name:{}, age:{}, intro:{}", id, email, strName, age, intro));
        Log::Debug("id:{}, email:{}, name:{}, age:{}, intro:{}", id, email, strName, age, intro);

        resp.SetStatusCode(Status::Ok);
    }
}