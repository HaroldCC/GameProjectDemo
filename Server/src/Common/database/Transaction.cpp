/*************************************************************************
> File Name       : Transaction.cpp
> Brief           : 事务
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年09月02日  15时28分01秒
************************************************************************/
#include "pch.h"
#include "Transaction.h"

TransactionBase::TransactionBase()
    : _cleanUp(false)
{
}

TransactionBase::~TransactionBase()
{
    CleanUp();
}

void TransactionBase::Append(std::string_view sql)
{
    _queries.emplace_back(sql);
}

size_t TransactionBase::GetSize() const
{
    return _queries.size();
}

void TransactionBase::AppendPreparedStatement(PreparedStatementBase *pStmt)
{
    _queries.emplace_back(pStmt);
}

void TransactionBase::CleanUp()
{
    // 防止虚构函数引起的二次清理
    if (_cleanUp)
    {
        return;
    }

    for (const auto &item : _queries)
    {
        if (std::holds_alternative<PreparedStatementBase *>(item))
        {
            PreparedStatementBase *pStmt = std::get<PreparedStatementBase *>(item);
            delete pStmt;
        }
    }

    _queries.clear();
    _cleanUp = true;
}
