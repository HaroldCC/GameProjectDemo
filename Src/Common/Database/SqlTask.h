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
#include "Common/Database/PreparedStatement.h"

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
class PreparedQueryTask final : public ISqlTask
{
public:
    explicit PreparedQueryTask(PreparedStatementBase *pStmt, bool async = false);
    ~PreparedQueryTask() override;

    PreparedQueryTask(const PreparedQueryTask &)            = delete;
    PreparedQueryTask(PreparedQueryTask &&)                 = delete;
    PreparedQueryTask &operator=(const PreparedQueryTask &) = delete;
    PreparedQueryTask &operator=(PreparedQueryTask &&)      = delete;

    bool                               Execute() override;
    [[nodiscard]] PreparedResultFuture GetFuture() const;

private:
    PreparedStatementBase *_pStmt;
    bool                   _hasResult;
    PreparedResultPromise *_pPreparedResultPromise;
};

// 数据库事务执行任务
class TransactionBase;

class TransactionTask : public ISqlTask
{
public:
    explicit TransactionTask(std::shared_ptr<TransactionBase> pTransaction);
    ~TransactionTask() override = default;

    TransactionTask(const TransactionTask &)            = delete;
    TransactionTask(TransactionTask &&)                 = delete;
    TransactionTask &operator=(const TransactionTask &) = delete;
    TransactionTask &operator=(TransactionTask &&)      = delete;

    bool     Execute() override;
    uint32_t TryExecute();
    void     CleanUpOnFailure();

private:
    std::shared_ptr<TransactionBase> _pTransaction;
    static std::mutex                _mutex;
};

// 数据库事务执行带结果任务
class TransactionWithResultTask final : public TransactionTask
{
public:
    explicit TransactionWithResultTask(std::shared_ptr<TransactionBase> pTransaction);

    TransactionFuture GetFuture();

    bool Execute() override;

private:
    TransactionPromise _transactionPromise;
};
