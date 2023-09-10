/*************************************************************************
> File Name       : Transaction.h
> Brief           : 事务
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年09月02日  11时35分38秒
************************************************************************/
#pragma once
#include "PreparedStatement.h"

#include <vector>
#include <variant>
#include <string_view>
#include "DatabaseEnv.h"

class PreparedStatementBase;

class TransactionBase
{
    friend class TransactionTask;
    friend class IMySqlConnection;

    template <typename ConnectionType>
    friend class DatabaseWorkerPool;

public:
    TransactionBase();

    TransactionBase(const TransactionBase &)            = default;
    TransactionBase(TransactionBase &&)                 = default;
    TransactionBase &operator=(const TransactionBase &) = default;
    TransactionBase &operator=(TransactionBase &&)      = default;

    virtual ~TransactionBase();

    void Append(std::string_view sql);

    [[nodiscard]] size_t GetSize() const;

protected:
    void AppendPreparedStatement(PreparedStatementBase *pStmt);
    void CleanUp();

protected:
    using SqlElementData = std::variant<std::string_view, PreparedStatementBase *>;
    std::vector<SqlElementData> _queries;

private:
    bool _cleanUp;
};

template <typename ConnType>
class Transaction : public TransactionBase
{
public:
    using TransactionBase::Append;

    void Append(PreparedStatement<ConnType> *pStmt)
    {
        AppendPreparedStatement(pStmt);
    }
};

class TransactionCallback
{
public:
    explicit TransactionCallback(TransactionFuture &&future);
    TransactionCallback(TransactionCallback &&)            = default;
    TransactionCallback &operator=(TransactionCallback &&) = default;
    ~TransactionCallback()                                 = default;

    TransactionCallback(const TransactionCallback &)            = delete;
    TransactionCallback &operator=(const TransactionCallback &) = delete;

    void Then(std::function<void(bool)> callback) &;

    bool InvokeIfReady();

private:
    TransactionFuture         _future;
    std::function<void(bool)> _callback;
};