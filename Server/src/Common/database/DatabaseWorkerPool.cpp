/*************************************************************************
> File Name       : DatabaseWorkerPool.cpp
> Brief           : 数据库工作池
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年08月03日  15时30分14秒
************************************************************************/
#include "pch.h"
#include "DatabaseWorkerPool.h"
#include "Common/include/Assert.h"
#include "MysqlConnection.h"
#include "QueryCallBack.h"
#include "SqlTask.h"
#include "Transaction.h"

template <typename ConnectionType>
DatabaseWorkerPool<ConnectionType>::DatabaseWorkerPool()
    : _queue(new ProducerConsumerQueue<ISqlTask *>()),
      _asyncThreadCount(0), _syncThreadCount(0)
{
    Assert(mysql_thread_safe(), "数据库不是线程安全的");
}

template <typename ConnectionType>
DatabaseWorkerPool<ConnectionType>::~DatabaseWorkerPool()
{
    _queue->Clear();
}

template <typename ConnectionType>
uint32_t DatabaseWorkerPool<ConnectionType>::Open(const MySqlConnectionInfo &info, uint8_t syncThreadCount, uint8_t asyncThreadCount)
{
    _pConnectionInfo  = std::make_unique<MySqlConnectionInfo>(info);
    _syncThreadCount  = syncThreadCount;
    _asyncThreadCount = asyncThreadCount;
    Log::Info("开始连接数据库：{} 同步方式连接数{}, 异步方式连接数{}", info._database, _syncThreadCount, _asyncThreadCount);

    uint32_t errcode = OpenConnections(EConnectionTypeIndex_Async, _asyncThreadCount);
    if (0 != errcode)
    {
        return errcode;
    }

    errcode = OpenConnections(EConnectionTypeIndex_Sync, _syncThreadCount);
    if (0 == errcode)
    {
        Log::Info("连接数据库：{} 成功。当前共{}个连接", info._database,
                  (_connections[EConnectionTypeIndex_Sync].size() + _connections[EConnectionTypeIndex_Async].size()));
    }

    return errcode;
}

template <typename ConnectionType>
void DatabaseWorkerPool<ConnectionType>::Close()
{
    Log::Info("数据库:{}关闭", _pConnectionInfo->_database);

    _connections[EConnectionTypeIndex_Async].clear();
    _connections[EConnectionTypeIndex_Sync].clear();
}

template <typename ConnectionType>
QueryCallback DatabaseWorkerPool<ConnectionType>::AsyncQuery(std::string_view sql)
{
    ADHOCQueryTask   *pTask        = new ADHOCQueryTask(sql, true);
    QueryResultFuture resultFuture = pTask->GetFuture();
    _queue->Push(pTask);
    return QueryCallback(std::move(resultFuture));
}

template <typename ConnectionType>
QueryCallback DatabaseWorkerPool<ConnectionType>::AsyncQuery(PreparedStatement<ConnectionType> *pStmt)
{
    PreparedQueryTask   *pTask        = new PreparedQueryTask(pStmt, true);
    PreparedResultFuture resultFuture = pTask->GetFuture();
    _queue->Push(pTask);
    return QueryCallback(std::move(resultFuture));
}

template <typename ConnectionType>
ResultSetPtr DatabaseWorkerPool<ConnectionType>::SyncQuery(std::string_view sql)
{
    ConnectionType *pConnection = GetFreeConnection();
    Assert(nullptr != pConnection);

    ResultSetPtr pResult = pConnection->Query(sql);
    pConnection->Unlock();

    if (nullptr == pResult || 0 == pResult->GetRowCount() || !pResult->NextRow())
    {
        pResult.reset();
        return pResult;
    }

    return pResult;
}

template <typename ConnectionType>
PreparedResultSetPtr DatabaseWorkerPool<ConnectionType>::SyncQuery(PreparedStatement<ConnectionType> *pStmt)
{
    ConnectionType      *pConnection = GetFreeConnection();
    PreparedResultSetPtr pResult     = pConnection->Query(pStmt);
    pConnection->Unlock();

    // 清空代理语句内存
    delete pStmt;
    pStmt = nullptr;

    if (nullptr == pResult || 0 == pResult->GetRowCount())
    {
        pResult.reset();
        return pResult;
    }

    return pResult;
}

template <typename ConnectionType>
void DatabaseWorkerPool<ConnectionType>::BeginTransaction()
{
    return std::make_shared<Transaction<ConnectionType>>();
}

template <typename ConnectionType>
void DatabaseWorkerPool<ConnectionType>::CommitTransaction(TransactionPtr<ConnectionType> pTransaction)
{
    _queue->Push(new TransactionTask(pTransaction));
}

template <typename ConnectionType>
uint32_t DatabaseWorkerPool<ConnectionType>::OpenConnections(EConnectionTypeIndex connType, uint8_t connectionCount)
{
    for (uint8_t i = 0; i < connectionCount; ++i)
    {
        ConnectionPtr pConnection = nullptr;
        switch (connType)
        {
        case EConnectionTypeIndex_Async:
        {
            pConnection = std::make_unique<ConnectionType>(_queue.get(), *_pConnectionInfo);
        }
        break;
        case EConnectionTypeIndex_Sync:
        {
            pConnection = std::make_unique<ConnectionType>(*_pConnectionInfo);
        }
        break;
        default:
            Assert(false, std::format("未知的连接类型索引：{}，请检查配置，服务器退出...", connType));
        };

        Assert(pConnection != nullptr);

        if (uint32_t errcode = pConnection->Open())
        {
            _connections[connType].clear();
            return errcode;
        }

        _connections[connType].emplace_back(std::move(pConnection));
    }

    return 0;
}

template <typename ConnectionType>
ConnectionType *DatabaseWorkerPool<ConnectionType>::GetFreeConnection()
{
    const auto      connectionCount = _connections[EConnectionTypeIndex_Sync].size();
    ConnectionType *pConnection     = nullptr;
    uint8_t         index           = 0;
    while (nullptr == pConnection)
    {
        pConnection = _connections[EConnectionTypeIndex_Sync][index++ % connectionCount].get();
        if (pConnection->TryLock())
        {
            break;
        }
    }

    return pConnection;
}
