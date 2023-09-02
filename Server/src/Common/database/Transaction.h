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

class PreparedStatementBase;

class TransactionBase
{
public:
    TransactionBase();

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