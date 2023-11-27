/*************************************************************************
> File Name       : server.cpp
> Brief           : 服务器
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月06日  14时41分54秒
************************************************************************/
#include "Common/Util/Platform.h"
#include "Server.h"
#include "Common/Util/Log.hpp"
#include "Session.h"

namespace net
{

    static thread_local bool gNetThreadFlag = false; // 网络线程标志

    IServer::IServer(asio::io_context &ioContext, uint16_t port)
        : _ioContext(ioContext)
        , _accepter(ioContext, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
        , _updateTimer(_ioContext)
    {
    }

    void IServer::Start()
    {
        DoAccept();

        _updateTimer.expires_from_now(std::chrono::milliseconds(1));
        _updateTimer.async_wait([this](const std::error_code &errc) {
            Update();
        });

        _netThread = std::thread([this]() {
            gNetThreadFlag = true;
            while (true)
            {
                try
                {
                    _ioContext.run();
                    break; // 正常退出
                }
                catch (const std::exception &e)
                {
                    Log::Error("服务器异常退出:{}", e.what());
                }
                catch (...)
                {
                    Log::Critical("服务器未知异常！！！！！");
                }
            }
        });
    }

    void IServer::Update()
    {
        _updateTimer.expires_from_now(std::chrono::milliseconds(1));
        _updateTimer.async_wait([this](const std::error_code &errc) {
            Update();
        });

        // 处理回调
        _processor.ProcessReadyCallbacks();

        _sessions.erase(std::remove_if(_sessions.begin(),
                                       _sessions.end(),
                                       [](std::shared_ptr<ISession> pSession) {
                                           if (!pSession->Update())
                                           {
                                               pSession->CloseSession();
                                               return true;
                                           }

                                           return false;
                                       }),
                        _sessions.end());
    }

    // void Server::DoAccept()
    // {
    //     _accepter.async_accept(
    //         [this](const std::error_code &errcode, asio::ip::tcp::socket socket)
    //         {
    //             if (errcode)
    //             {
    //                 Log::error("接受连接失败：{}", errcode.message());
    //                 return;
    //             }

    //             auto pSession = std::make_shared<Session>(std::move(socket));
    //             pSession->StartSession();

    //             DoAccept();
    //         });
    // }
} // namespace net