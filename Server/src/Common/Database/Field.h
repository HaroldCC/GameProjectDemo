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
#include <vector>
#include "MysqlTypeDef.h"
#include <any>

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

template <typename T>
struct MysqlType
{
};

template <>
struct MysqlType<uint8_t> : std::integral_constant<enum_field_types, MYSQL_TYPE_TINY>
{
};

template <>
struct MysqlType<uint16_t> : std::integral_constant<enum_field_types, MYSQL_TYPE_SHORT>
{
};

template <>
struct MysqlType<uint32_t> : std::integral_constant<enum_field_types, MYSQL_TYPE_LONG>
{
};

template <>
struct MysqlType<uint64_t> : std::integral_constant<enum_field_types, MYSQL_TYPE_LONGLONG>
{
};

template <>
struct MysqlType<int8_t> : std::integral_constant<enum_field_types, MYSQL_TYPE_TINY>
{
};

template <>
struct MysqlType<int16_t> : std::integral_constant<enum_field_types, MYSQL_TYPE_SHORT>
{
};

template <>
struct MysqlType<int32_t> : std::integral_constant<enum_field_types, MYSQL_TYPE_LONG>
{
};

template <>
struct MysqlType<int64_t> : std::integral_constant<enum_field_types, MYSQL_TYPE_LONGLONG>
{
};

template <>
struct MysqlType<float> : std::integral_constant<enum_field_types, MYSQL_TYPE_FLOAT>
{
};

template <>
struct MysqlType<double> : std::integral_constant<enum_field_types, MYSQL_TYPE_DOUBLE>
{
};

template <DatabaseFieldType>
struct FieldValueType
{
};
template <>
struct FieldValueType<DatabaseFieldType::UInt8>
{
    using type = uint8_t;
};

template <>
struct FieldValueType<DatabaseFieldType::Double>
{
    using type = double;
};

// 查询结果字段信息
struct QueryResultFieldMetadata
{
    std::string_view  tableName;
    std::string_view  tableAlias;
    std::string_view  name;
    std::string_view  alias;
    std::string_view  typeName;
    uint32_t          index     = 0;
    DatabaseFieldType fieldType = DatabaseFieldType::Null;
};

class ResultSet;

/**
    @class Field

    @brief Class used to access individual fields of database query result

    Guideline on field type matching:

    |   MySQL type           |  method to use                         |
    |------------------------|----------------------------------------|
    | TINYINT                | GetBool, GetInt8, GetUInt8             |
    | SMALLINT               | GetInt16, GetUInt16                    |
    | MEDIUMINT, INT         | GetInt32, GetUInt32                    |
    | BIGINT                 | GetInt64, GetUInt64                    |
    | FLOAT                  | GetFloat                               |
    | DOUBLE, DECIMAL        | GetDouble                              |
    | CHAR, VARCHAR,         | GetCString, GetString                  |
    | TINYTEXT, MEDIUMTEXT,  | GetCString, GetString                  |
    | TEXT, LONGTEXT         | GetCString, GetString                  |
    | TINYBLOB, MEDIUMBLOB,  | GetBinary, GetString                   |
    | BLOB, LONGBLOB         | GetBinary, GetString                   |
    | BINARY, VARBINARY      | GetBinary                              |

    Return types of aggregate functions:

    | Function |       Type        |
    |----------|-------------------|
    | MIN, MAX | Same as the field |
    | SUM, AVG | DECIMAL           |
    | COUNT    | BIGINT            |
*/

class Field
{
    friend class ResultSet;
    friend class PreparedResultSet;

public:
    Field();
    Field(const Field &)            = default;
    Field &operator=(const Field &) = default;
    ~Field()                        = default;

    Field(Field &&)            = delete;
    Field &operator=(Field &&) = delete;

private:
    [[nodiscard]] bool GetBool() const // Wrapper, actually gets integer
    {
        return GetUInt8() == 1;
    }

    [[nodiscard]] uint8_t     GetUInt8() const;
    [[nodiscard]] int8_t      GetInt8() const;
    [[nodiscard]] uint16_t    GetUInt16() const;
    [[nodiscard]] int16_t     GetInt16() const;
    [[nodiscard]] uint32_t    GetUInt32() const;
    [[nodiscard]] int32_t     GetInt32() const;
    [[nodiscard]] uint64_t    GetUInt64() const;
    [[nodiscard]] int64_t     GetInt64() const;
    [[nodiscard]] float       GetFloat() const;
    [[nodiscard]] double      GetDouble() const;
    [[nodiscard]] char const *GetCString() const;
    // [[nodiscard]] std::string          GetString() const;
    // [[nodiscard]] std::string_view     GetStringView() const;
    // [[nodiscard]] std::vector<uint8_t> GetBinary() const;
    // template <size_t S>
    // std::array<uint8_t, S> GetBinary() const
    // {
    //     std::array<uint8_t, S> buf;
    //     GetBinarySizeChecked(buf.data(), S);
    //     return buf;
    // }

    [[nodiscard]] bool IsNull() const
    {
        return _pValue == nullptr;
    }

public:
    [[nodiscard]] DatabaseFieldType GetType() const;

    [[nodiscard]] bool IsType(DatabaseFieldType type) const;

    explicit operator uint8_t()
    {
        return GetUInt8();
    }

    explicit operator uint16_t()
    {
        return GetUInt16();
    }

    explicit operator uint32_t()
    {
        return GetUInt32();
    }

    explicit operator uint64_t()
    {
        return GetUInt64();
    }

    explicit operator int8_t()
    {
        return GetInt8();
    }

    explicit operator int16_t()
    {
        return GetInt16();
    }

    explicit operator int32_t()
    {
        return GetInt32();
    }

    explicit operator int64_t()
    {
        return GetInt64();
    }

    explicit operator const char *()
    {
        return GetCString();
    }

private:
    void LogWrongGetter(const char *getter) const;

    void SetValue(char const *newValue, uint32_t length);

    void SetMetadata(const QueryResultFieldMetadata *fieldMeta);

    void GetBinarySizeChecked(uint8_t *buf, size_t size) const;

private:
    const char                     *_pValue;
    uint32_t                        _length;
    const QueryResultFieldMetadata *_meta;
};
