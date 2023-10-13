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
#include "QueryCallback.h"
#include "SqlTask.h"
#include "Transaction.h"
#include "MySqlTypeHack.h"
#include "QueryResult.h"
#include "MySqlPreparedStatement.h"
#include "LoginDatabase.h"
#include "Common/include/Util.hpp"

template <typename ConnectionType>
DatabaseWorkerPool<ConnectionType>::DatabaseWorkerPool()
    : _queue(new ProducerConsumerQueue<ISqlTask *>())
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
bool DatabaseWorkerPool<ConnectionType>::PrepareStatements()
{
    for (auto &&connectionType : _connections)
    {
        for (auto &&pConnection : connectionType)
        {
            pConnection->TryLock();
            if (!pConnection->PrepareStatements())
            {
                pConnection->UnLock();
                Close();
                return false;
            }

            pConnection->UnLock();

            const size_t preparedSize = pConnection->_stmts.size();
            if (_preparedStmtParamCount.size() < preparedSize)
            {
                _preparedStmtParamCount.resize(preparedSize);
            }

            for (size_t i = 0; i < preparedSize; ++i)
            {
                if (_preparedStmtParamCount[i] > 0)
                {
                    continue;
                }

                if (MySqlPreparedStatement *pStmt = pConnection->_stmts[i].get(); pStmt != nullptr)
                {
                    const uint32_t paramCount = pStmt->GetParameterCount();
                    Assert(paramCount < (std::numeric_limits<uint8_t>::max)());

                    _preparedStmtParamCount[i] = static_cast<uint8_t>(paramCount);
                }
            }
        }
    }

    return true;
}

template <typename ConnectionType>
void DatabaseWorkerPool<ConnectionType>::AsyncExecute(std::string_view sql)
{
    ADHOCQueryTask *pTask = new ADHOCQueryTask(sql);
    _queue->Push(pTask);
}

template <typename ConnectionType>
void DatabaseWorkerPool<ConnectionType>::AsyncExecute(PreparedStatement<ConnectionType> *pStmt)
{
    PreparedQueryTask *pTask = new PreparedQueryTask(pStmt);
    _queue->Push(pTask);
}

template <typename ConnectionType>
void DatabaseWorkerPool<ConnectionType>::SyncExecute(std::string_view sql)
{
    ConnectionType *pConnection = GetFreeConnectionAndLock();
    pConnection->Execute(sql);
    pConnection->UnLock();
}

template <typename ConnectionType>
void DatabaseWorkerPool<ConnectionType>::SyncExecute(PreparedStatement<ConnectionType> *pStmt)
{
    ConnectionType *pConnection = GetFreeConnectionAndLock();
    pConnection->Execute(pStmt);
    pConnection->UnLock();
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
    ConnectionType *pConnection = GetFreeConnectionAndLock();
    Assert(nullptr != pConnection);

    ResultSetPtr pResult = pConnection->Query(sql);
    pConnection->UnLock();

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
    ConnectionType      *pConnection = GetFreeConnectionAndLock();
    PreparedResultSetPtr pResult     = pConnection->Query(pStmt);
    pConnection->UnLock();

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
TransactionPtr<ConnectionType> DatabaseWorkerPool<ConnectionType>::BeginTransaction()
{
    return std::make_shared<Transaction<ConnectionType>>();
}

template <typename ConnectionType>
void DatabaseWorkerPool<ConnectionType>::CommitTransaction(TransactionPtr<ConnectionType> pTransaction)
{
    _queue->Push(new TransactionTask(pTransaction));
}

template <typename ConnectionType>
TransactionCallback DatabaseWorkerPool<ConnectionType>::CommitTransactionWithCallback(TransactionPtr<ConnectionType> pTransaction)
{
    TransactionWithResultTask *pTask  = new TransactionWithResultTask(pTransaction);
    TransactionFuture          future = pTask->GetFuture();
    _queue->Push(pTask);
    return TransactionCallback(std::move(future));
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
            Assert(false, std::format("未知的连接类型索引:{}，请检查配置，服务器退出...", Util::ToUnderlying(connType)));
            break;
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
ConnectionType *DatabaseWorkerPool<ConnectionType>::GetFreeConnectionAndLock()
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

template <typename ConnectionType>
PreparedStatement<ConnectionType> *DatabaseWorkerPool<ConnectionType>::GetPreparedStatement(uint32_t index)
{
    return new PreparedStatement<ConnectionType>(index, _preparedStmtParamCount[index]);
}

template class DatabaseWorkerPool<LoginDatabaseConnection>;
