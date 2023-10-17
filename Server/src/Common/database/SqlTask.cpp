/*************************************************************************
> File Name       : SqlTask.cpp
> Brief           : 数据库操作
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年09月02日  09时10分29秒
************************************************************************/
#include "SqlTask.h"

#include "MysqlConnection.h"
#include "QueryResult.h"
#include "mysqld_error.h"
#include "Transaction.h"
#include "Common/include/Performance.hpp"

bool PingTask::Execute()
{
    _pSqlConn->Ping();
    return true;
}

ADHOCQueryTask::ADHOCQueryTask(std::string_view sql, bool async /* = false */)
    : _sql(sql), _hasResult(async), _pQueryResultPromise(nullptr)
{
    if (_hasResult)
    {
        _pQueryResultPromise = new QueryResultPromise();
    }
}

ADHOCQueryTask::~ADHOCQueryTask()
{
    if (_hasResult && nullptr != _pQueryResultPromise)
    {
        delete _pQueryResultPromise;
        _pQueryResultPromise = nullptr;
    }
}

bool ADHOCQueryTask::Execute()
{
    if (_hasResult)
    {
        ResultSetPtr pResult = _pSqlConn->Query(_sql);
        if (nullptr == pResult || pResult->GetRowCount() == 0 || !pResult->NextRow())
        {
            pResult.reset();
            _pQueryResultPromise->set_value(nullptr);
            return false;
        }

        _pQueryResultPromise->set_value(pResult);
        return true;
    }

    return _pSqlConn->Execute(_sql);
}

QueryResultFuture ADHOCQueryTask::GetFuture() const
{
    return _pQueryResultPromise->get_future();
}

PreparedQueryTask::PreparedQueryTask(PreparedStatementBase *pStmt, bool async /* = false */)
    : _pStmt(pStmt), _hasResult(async), _pPreparedResultPromise(nullptr)
{
    if (_hasResult)
    {
        _pPreparedResultPromise = new PreparedResultPromise();
    }
}

PreparedQueryTask::~PreparedQueryTask()
{
    delete _pStmt;
    _pStmt = nullptr;
    if (_hasResult && nullptr != _pPreparedResultPromise)
    {
        delete _pPreparedResultPromise;
        _pPreparedResultPromise = nullptr;
    }
}

bool PreparedQueryTask::Execute()
{
    if (_hasResult)
    {
        PreparedResultSetPtr pResult = _pSqlConn->Query(_pStmt);
        if (nullptr == pResult || pResult->GetRowCount() == 0)
        {
            pResult.reset();
            _pPreparedResultPromise->set_value(nullptr);
            return false;
        }

        _pPreparedResultPromise->set_value(pResult);
        return true;
    }

    return _pSqlConn->Execute(_pStmt);
}

PreparedResultFuture PreparedQueryTask::GetFuture() const
{
    return _pPreparedResultPromise->get_future();
}

std::mutex TransactionTask::_mutex;

TransactionTask::TransactionTask(std::shared_ptr<TransactionBase> pTransaction)
    : _pTransaction(std::move(pTransaction))
{
}

bool TransactionTask::Execute()
{
    uint32_t errcode = TryExecute();
    if (errcode == 0)
    {
        return true;
    }

    if (errcode == ER_LOCK_DEADLOCK)
    {
        std::ostringstream strStream;
        strStream << std::this_thread::get_id();
        auto threadID = strStream.view();

        std::lock_guard<std::mutex> lock(_mutex);
        constexpr uint64_t          maxRetryTimeMillSec = 60'000;
        for (uint64_t loopDuration = 0, appStartTime = Util::GetMillSecTimeNow();
             loopDuration <= maxRetryTimeMillSec; loopDuration = Util::GetMillSecTimeDiffToNow(appStartTime))
        {
            if (TryExecute() == 0)
            {
                return true;
            }

            Log::Warn("执行事务死锁，重新执行中...重试次数：{} 线程ID：{}", loopDuration, threadID);
        }

        Log::Error("执行事务死锁失败，不再重新执行，线程ID：{}", threadID);
    }

    CleanUpOnFailure();
    return false;
}

uint32_t TransactionTask::TryExecute()
{
    return _pSqlConn->ExecuteTransaction(_pTransaction);
}

void TransactionTask::CleanUpOnFailure()
{
    _pTransaction->CleanUp();
}

TransactionWithResultTask::TransactionWithResultTask(std::shared_ptr<TransactionBase> pTransaction)
    : TransactionTask(std::move(pTransaction))
{
}

TransactionFuture TransactionWithResultTask::GetFuture()
{
    return _transactionPromise.get_future();
}

bool TransactionWithResultTask::Execute()
{
    bool bResult = TransactionTask::Execute();
    _transactionPromise.set_value(bResult);

    return bResult;
}