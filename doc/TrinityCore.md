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

# 数据库模块
``` mermaid
classDiagram
class MySQLConnection{
    <<abstract>>
    + MySQLConnection(connInfo : MySqlConnectionInfo &);
    + MySQLConnection(queue : ProducerConsumerQueue<SQLOperation*>*, connInfo : MySQLConnection&);
    # void DoPrepareStatements()*;
}

class LoginDatabaseConnection{
    # void DoPrepareStatements();
}

class CharacterDatabaseConnection{
    # void DoPrepareStatements();
}

class WorldDatabaseConnection{
    # void DoPrepareStatements();
}

MySQLConnection <|-- LoginDatabaseConnection
MySQLConnection <|-- CharacterDatabaseConnection
MySQLConnection <|-- WorldDatabaseConnection

class SQLOperation{
    + bool Execute()*;
}

class PingOperation{
    - bool Execute();
}

class SQLQueryHolderTask{
    + bool Execute();
}

class BasicStatementTask{
    + bool Execute();
}

class TransactionTask{
    # bool Execute();
}

class TransactionWithResultTask{
    # bool Execute();
}

class PreparedStatementTask{
    + bool Execute();
}

SQLOperation <|-- PingOperation
SQLOperation <|-- SQLQueryHolderTask
SQLOperation <|-- BasicStatementTask
SQLOperation <|-- TransactionTask
TransactionTask <|-- TransactionWithResultTask
SQLOperation <|-- PreparedStatementTask

SQLOperation "1" --> "1" MySQLConnection : contains

class SQLQueryHolderBase

class SQLQueryHolder~T~

SQLQueryHolderBase <|-- SQLQueryHolder
SQLQueryHolderTask "1" --> "1" SQLQueryHolderBase : uses

class DatabaseWorker{
    + DatabaseWorker(newQueue : ProducerConsumerQueue<SQLOperation*>*, connection : MySQLConnection);
    - _workerThread : thread
}

class DatabaseWorkerPool~T~{
    - uint32_t OpenConnections(type : enum, numConnections : uint8_t);
}


class ProducerConsumerQueue~T~


DatabaseWorker "1" --> "1" ProducerConsumerQueue~T~ : uses
ProducerConsumerQueue "1" --> "*" SQLOperation : uses

DatabaseWorkerPool "1" --> "*" ProducerConsumerQueue : contains

class DatabaseLoader{
    + DatabaseLoader& AddDatabase~T~(pool : DatabaseWorkerPool, name : string);
    + void Load();
    - _open     : queue
    - _populate : queue
    - _update   : queue
    - _prepare  : queue
}
```

# 测试
@import "TrinityCore_database.plantuml"