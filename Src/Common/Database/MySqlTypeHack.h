/*************************************************************************
> File Name       : MySqlTypeHack.h
> Brief           : Mysql类型别名
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年09月15日  17时27分11秒
************************************************************************/

#include <type_traits>
#include <mysql.h>
#include "Field.h"

using MySqlHandle = MYSQL;
using MySqlResult = MYSQL_RES;
using MySqlField  = MYSQL_FIELD;
using MySqlBind   = MYSQL_BIND;
using MySqlStmt   = MYSQL_STMT;

// Mysql 8 去除了my_bool 类型的typedef，使用这个来保证前后的兼容性，（learn by TrinityCore source）
using MySqlBool = std::remove_pointer_t<decltype(std::declval<MYSQL_BIND>().is_null)>;

template <typename T>
struct MysqlType
{
};

template <>
struct MysqlType<uint8_t>
    : std::integral_constant<enum_field_types, MYSQL_TYPE_TINY>
{
};

template <>
struct MysqlType<uint16_t>
    : std::integral_constant<enum_field_types, MYSQL_TYPE_SHORT>
{
};

template <>
struct MysqlType<uint32_t>
    : std::integral_constant<enum_field_types, MYSQL_TYPE_LONG>
{
};

template <>
struct MysqlType<uint64_t>
    : std::integral_constant<enum_field_types, MYSQL_TYPE_LONGLONG>
{
};

template <>
struct MysqlType<int8_t>
    : std::integral_constant<enum_field_types, MYSQL_TYPE_TINY>
{
};

template <>
struct MysqlType<int16_t>
    : std::integral_constant<enum_field_types, MYSQL_TYPE_SHORT>
{
};

template <>
struct MysqlType<int32_t>
    : std::integral_constant<enum_field_types, MYSQL_TYPE_LONG>
{
};

template <>
struct MysqlType<int64_t>
    : std::integral_constant<enum_field_types, MYSQL_TYPE_LONGLONG>
{
};

template <>
struct MysqlType<float> : std::integral_constant<enum_field_types, MYSQL_TYPE_FLOAT>
{
};

template <>
struct MysqlType<double>
    : std::integral_constant<enum_field_types, MYSQL_TYPE_DOUBLE>
{
};
