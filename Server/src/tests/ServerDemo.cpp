#include "Common/net/server.h"
#include "Common/include/log.hpp"

int main()
{
    Log::CLogger::GetLogger().InitLogger("log/ServerLog.html", 0, 10240, 10);

    asio::io_context ioContext;

    net::Server server(ioContext, 10005);
    server.Start();

    ioContext.run();

    return 0;
}
