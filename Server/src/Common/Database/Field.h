/*************************************************************************
> File Name       : Field.h
> Brief           : 数据库表字段信息
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年08月11日  17时51分44秒
************************************************************************/
#pragma once

#include <cstdint>
#include <string_view>

// 数据库字段类型
enum class DatabaseFieldType : uint8_t
{
    Null,
    Int8,
    Int16,
    Int32,
    Int64,
    Float,
    Double,
    Decimal,
    Date,
    Binary
};

// 查询结果字段信息
struct QueryResultFieldFieldMetadata
{
    std::string_view  tableName;
    std::string_view  tableAlias;
    std::string_view  name;
    std::string_view  alias;
    std::string_view  typeName;
    uint32_t          index     = 0;
    DatabaseFieldType fieldType = DatabaseFieldType::Null;
};