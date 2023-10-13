#include "HttpServer.h"
#include "Common/include/Log.hpp"
#include "Common/database/LoginDatabase.h"

int main()
{
    Log::CLogger::GetLogger().InitLogger("log/HttpServer.html", 0, 10240, 10);

    g_LoginDatabase.Open({"root", "cr11234", "test", "127.0.0.1", "3306"}, 1, 1);
    g_LoginDatabase.PrepareStatements();

    asio::io_context ioContext;
    try
    {
        HttpServer server(ioContext, 10006);
        server.Start();
        ioContext.run();
    }
    catch (const std::exception &exception)
    {
        Log::Critical("Http服务器发生异常：{}", exception.what());
    }
    catch (...)
    {
        Log::Critical("Http服务器发生未知异常!!!");
    }

    return 0;
}