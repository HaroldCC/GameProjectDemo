/*************************************************************************
> File Name       : DatabaseWorkerPool.h
> Brief           : 数据库工作池
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年08月03日  15时27分43秒
************************************************************************/
#pragma once
#include <cstdint>
#include <string_view>
#include <array>
#include <memory>
#include "Common/Threading/ProducerConsumerQueue.hpp"
#include "Transaction.h"

struct MySqlConnectionInfo;

class ISqlTask;
class QueryCallback;
class TransactionBase;

template <typename ConnectionType>
class PreparedStatement;

template <typename ConnectionType>
class DatabaseWorkerPool
{
private:
    enum EConnectionTypeIndex
    {
        EConnectionTypeIndex_Async,
        EConnectionTypeIndex_Sync,
        EConnectionTypeIndex_Max,
    };

public:
    DatabaseWorkerPool();
    ~DatabaseWorkerPool();

    DatabaseWorkerPool(const DatabaseWorkerPool &)            = delete;
    DatabaseWorkerPool(DatabaseWorkerPool &&)                 = delete;
    DatabaseWorkerPool &operator=(const DatabaseWorkerPool &) = delete;
    DatabaseWorkerPool &operator=(DatabaseWorkerPool &&)      = delete;

    uint32_t Open(const MySqlConnectionInfo &info, uint8_t syncThreadCount, uint8_t asyncThreadCount);
    void     Close();

    bool PrepareStatements();

    void AsyncExecute(std::string_view sql);
    void AsyncExecute(PreparedStatement<ConnectionType> *pStmt);

    void SyncExecute(std::string_view sql);
    void SyncExecute(PreparedStatement<ConnectionType> *pStmt);

    QueryCallback AsyncQuery(std::string_view sql);
    QueryCallback AsyncQuery(PreparedStatement<ConnectionType> *pStmt);

    ResultSetPtr         SyncQuery(std::string_view sql);
    PreparedResultSetPtr SyncQuery(PreparedStatement<ConnectionType> *pStmt);

    TransactionPtr<ConnectionType> BeginTransaction();
    void                           CommitTransaction(TransactionPtr<ConnectionType> pTransaction);
    TransactionCallback            CommitTransactionWithCallback(TransactionPtr<ConnectionType> pTransaction);

    PreparedStatement<ConnectionType> *GetPreparedStatement(uint32_t index);

private:
    uint32_t        OpenConnections(EConnectionTypeIndex connType, uint8_t connectionCount);
    ConnectionType *GetFreeConnectionAndLock();

private:
    using ConnectionPtr = std::unique_ptr<ConnectionType>;
    std::array<std::vector<ConnectionPtr>, EConnectionTypeIndex_Max> _connections;

    std::unique_ptr<MySqlConnectionInfo>               _pConnectionInfo;
    std::unique_ptr<ProducerConsumerQueue<ISqlTask *>> _queue; // 与异步线程（async worker）共享
    std::vector<uint8_t> _preparedStmtParamCount;              // 记录每个预处理语句参数数量
    uint8_t              _asyncThreadCount {};
    uint8_t              _syncThreadCount {};
};