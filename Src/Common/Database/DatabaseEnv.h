/*************************************************************************
> File Name       : DatabaseEnv.h
> Brief           : 数据库声明
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月22日  13时53分17秒
************************************************************************/
#pragma once

#include <memory>
#include <future>
#include <type_traits>

using MySqlHandle = struct MYSQL;
using MySqlResult = struct MYSQL_RES;
using MySqlField  = struct MYSQL_FIELD;
using MySqlBind   = struct MYSQL_BIND;
using MySqlStmt   = struct MYSQL_STMT;

class ResultSet;
using ResultSetPtr = std::shared_ptr<ResultSet>;

template <typename... Args>
inline ResultSetPtr MakeResultSetPtr(Args &&...args)
{
    return std::make_shared<ResultSet>(std::forward<Args>(args)...);
}

using QueryResultFuture  = std::future<ResultSetPtr>;
using QueryResultPromise = std::promise<ResultSetPtr>;

class PreparedResultSet;
using PreparedResultSetPtr = std::shared_ptr<PreparedResultSet>;

template <typename... Args>
inline PreparedResultSetPtr MakePreparedResultSetPtr(Args &&...args)
{
    return std::make_shared<PreparedResultSet>(std::forward<Args>(args)...);
}

using PreparedResultFuture  = std::future<PreparedResultSetPtr>;
using PreparedResultPromise = std::promise<PreparedResultSetPtr>;

template <typename ConnectionType>
class Transaction;

template <typename ConnectionType>
using TransactionPtr = std::shared_ptr<Transaction<ConnectionType>>;

using TransactionFuture  = std::future<bool>;
using TransactionPromise = std::promise<bool>;
