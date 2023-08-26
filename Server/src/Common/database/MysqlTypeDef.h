/*************************************************************************
> File Name       : MysqlTypeDef.h
> Brief           : Mysql别名
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月22日  13时53分17秒
************************************************************************/
#pragma once

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