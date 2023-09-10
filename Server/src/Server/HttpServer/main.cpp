#include "HttpServer.h"
#include "Common/include/Log.hpp"

int main()
{
    Log::CLogger::GetLogger().InitLogger("log/HttpServer.html", 0, 10240, 10);

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