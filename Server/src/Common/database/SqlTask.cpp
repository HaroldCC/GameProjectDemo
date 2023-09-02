/*************************************************************************
> File Name       : SqlTask.cpp
> Brief           : 数据库操作
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年09月02日  09时10分29秒
************************************************************************/
#include "pch.h"
#include "SqlTask.h"
#include "MysqlConnection.h"
#include "QueryResult.h"

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

IPreparedQueryTask::IPreparedQueryTask(PreparedStatementBase *pStmt, bool async /* = false */)
    : _pStmt(pStmt), _hasResult(async), _pPreparedResultPromise(nullptr)
{
    if (_hasResult)
    {
        _pPreparedResultPromise = new PreparedResultPromise();
    }
}

IPreparedQueryTask::~IPreparedQueryTask()
{
    delete _pStmt;
    _pStmt = nullptr;
    if (_hasResult && nullptr != _pPreparedResultPromise)
    {
        delete _pPreparedResultPromise;
        _pPreparedResultPromise = nullptr;
    }
}

bool IPreparedQueryTask::Execute()
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