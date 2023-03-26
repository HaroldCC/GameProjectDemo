# TrinityCore 网络模块分析

``` mermaid
classDiagram
class Socket~T~{
    <<abstract>>
    -_socket : asio::ip::tcp::socket;
    -_remoteAddress : asio::ip::adress;
    -_remotePort : uint16_t;
    ...
    +Socket(asio::ip::tcp::socket && socket)
    +~Socket()
    + virtual void Start()*
    + virtual bool Update()
    + void AsyncRead()
    + void CloseSocket()
    # virtual void OnClose()
    # virtual void ReadHandler()*
    -void ReadHandlerInternal()
}
class WorldSocket{
    -_worldSessionLock : std:mutex
    -_overSpeedPings : uint32_t
    +WorldSocket(asio::ip::tcp::socket &&socket)
    +~WorldSocket()
    +void Start()
    +void Update()
    #void OnClose()
    #void ReadHandler()
}
Socket <|-- WorldSocket : 继承
class SocketMgr~SocketType~{
    #_accepter : AsyncAcceptor*
    #_threads : NetworkThread~SocketType~*
    #_threadCount : int32_t
    +virtual ~SocketMgr()
    +virtual bool StartNetwork(asio::io_context *ioContext, const std::string& strBindIP, uint_16 port, int nThreadCount)
    +virtual void StopNetWork()
    +void Wait()
    +virtual void OnSocketOpen(aiso::ip::tcp::socket &&sock, uint32_t nThreadIndex)
    +int32_t GetNetworkThreadCount()
    +uint32_t SelectThreadWithMinConnections()
    #SocketMgr()
    #virtual NetworkThread~SocketType~***CreateThreads**()
}
class WorldSocketMgr{
    using BaseSocketMgr = SocketMgr~WorldSocket~
    -_socketSystemSendBuffSize : int32_t
    -_socketApplicationSendBuffSize : int32_t
    -_tcpNoDelay : bool
}
SocketMgr <|-- WorldSocketMgr~WordSocket~ : 继承
class NetworkThread~SocketType~{
    using SocketContainer = std::vector~shared_ptr~SocketType~~

    -_connections : std::atomic~int32_t~
    -_stopped : std::atomic~bool~
    -_thread : std::thread*
    -_sockets : SocketContainer
    -_newSocketsLock : std::mutex
    -_ioContext : asio:io_context
    -_acceptSocket : asio::ip::tcp::socket
    -_updateTimer : asio::DeadlineTimer
    +NetWorldThread()
    +virtual  ~NetworkThread()
    +void Stop()
    +bool Start()
    +void Wait()
    +int32_t GetConnectionCount()
    +virtual void AddSocket(std::shared_ptr~SocketType~sock)
    +asio::ip::tcp::socket *GetSocketForAccept()
    #virtual void SocketAdded(std::shared_ptr~SocketType~ socket)
    #virtual void SocketRemoved(std::shared_ptr~SocketType~ socket)
    #void AddNewSockets()
    #void Run()
    #void Update()
}
SocketMgr~SocketType~ *-- NetworkThread~SocketType~
```