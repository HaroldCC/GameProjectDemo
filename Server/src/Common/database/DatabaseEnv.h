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
#include <mysql.h>

using MySqlHandle = MYSQL;
using MySqlResult = MYSQL_RES;
using MySqlField  = MYSQL_FIELD;
using MySqlBind   = MYSQL_BIND;
using MySqlStmt   = MYSQL_STMT;

// Mysql 8 去除了my_bool 类型的typedef，使用这个来保证前后的兼容性，（learn by TrinityCore source）
using MySqlBool = std::remove_pointer_t<decltype(std::declval<MYSQL_BIND>().is_null)>;

// 数据库值类型
// 数据库字段类型
enum class DatabaseFieldType : uint8_t
{
    Null,
    UInt8,
    Int8,
    UInt16,
    Int16,
    UInt32,
    Int32,
    UInt64,
    Int64,
    Float,
    Double,
    Decimal,
    Date,
    Binary
};

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