/*************************************************************************
> File Name       : SqlTask.h
> Brief           : 数据库操作
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月28日  11时48分25秒
************************************************************************/
#pragma once
#include <string_view>

#include "DatabaseEnv.h"
#include "database/PreparedStatement.h"

class IMySqlConnection;

class ISqlTask
{
public:
    ISqlTask()          = default;
    virtual ~ISqlTask() = default;

    ISqlTask(const ISqlTask &)            = delete;
    ISqlTask(ISqlTask &&)                 = delete;
    ISqlTask &operator=(const ISqlTask &) = delete;
    ISqlTask &operator=(ISqlTask &&)      = delete;

    virtual bool Execute() = 0;
    virtual void SetConnection(IMySqlConnection *sqlConn)
    {
        _pSqlConn = sqlConn;
    }

protected:
    IMySqlConnection *_pSqlConn = nullptr;
};

class PingTask final : public ISqlTask
{
public:
    bool Execute() override;
};

// ad-hoc 即席查询任务
class ADHOCQueryTask final : public ISqlTask
{
public:
    explicit ADHOCQueryTask(std::string_view sql, bool async = false);
    ~ADHOCQueryTask() override;

    ADHOCQueryTask(const ADHOCQueryTask &)            = delete;
    ADHOCQueryTask(ADHOCQueryTask &&)                 = delete;
    ADHOCQueryTask &operator=(const ADHOCQueryTask &) = delete;
    ADHOCQueryTask &operator=(ADHOCQueryTask &&)      = delete;

    bool Execute() override;

    [[nodiscard]] QueryResultFuture GetFuture() const;

private:
    std::string_view    _sql;
    bool                _hasResult;
    QueryResultPromise *_pQueryResultPromise;
};

// 预处理语句查询任务
class IPreparedQueryTask final : public ISqlTask
{
public:
    explicit IPreparedQueryTask(PreparedStatementBase *pStmt, bool async = false);
    ~IPreparedQueryTask() override;

    IPreparedQueryTask(const IPreparedQueryTask &)            = delete;
    IPreparedQueryTask(IPreparedQueryTask &&)                 = delete;
    IPreparedQueryTask &operator=(const IPreparedQueryTask &) = delete;
    IPreparedQueryTask &operator=(IPreparedQueryTask &&)      = delete;

    bool                               Execute() override;
    [[nodiscard]] PreparedResultFuture GetFuture() const;

private:
    PreparedStatementBase *_pStmt;
    bool                   _hasResult;
    PreparedResultPromise *_pPreparedResultPromise;
};

// 数据库事务执行任务
class TransactionTask : public ISqlTask
{
public:
    TransactionTask();

protected:
};
