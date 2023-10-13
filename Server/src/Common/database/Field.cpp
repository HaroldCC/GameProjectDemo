﻿/*************************************************************************
> File Name       : Field.cpp
> Brief           : 数据库表字段信息
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年08月14日  17时33分42秒
************************************************************************/
#include "pch.h"
#include "Field.h"
#include "Common/include/Assert.h"
#include "MySqlTypeHack.h"
#include "Common/include/Util.hpp"
#include <variant>

template <typename T>
constexpr std::pair<bool, T> Field::CheckType() const
{
    Assert(nullptr != _meta);

    std::variant<uint8_t, int8_t, uint16_t, int16_t, uint32_t, int32_t, uint64_t, int64_t, float, double> source;

    bool bBinary = _meta->bBinary;
    switch (_meta->fieldType)
    {
    case DatabaseFieldType::Null:
        // source = nullptr;
        break;
    case DatabaseFieldType::UInt8:
        source = bBinary ? *reinterpret_cast<const uint8_t *>(_pValue) : Util::StringTo<uint8_t>({_pValue, _length}).value_or(0);
        break;
    case DatabaseFieldType::Int8:
        source = bBinary ? *reinterpret_cast<const int8_t *>(_pValue) : Util::StringTo<int8_t>({_pValue, _length}).value_or(0);
        break;
    case DatabaseFieldType::UInt16:
        source = bBinary ? *reinterpret_cast<const uint16_t *>(_pValue) : Util::StringTo<uint16_t>({_pValue, _length}).value_or(0);
        break;
    case DatabaseFieldType::Int16:
        source = bBinary ? *reinterpret_cast<const int16_t *>(_pValue) : Util::StringTo<int16_t>({_pValue, _length}).value_or(0);
        break;
    case DatabaseFieldType::UInt32:
        source = bBinary ? *reinterpret_cast<const uint32_t *>(_pValue) : Util::StringTo<uint32_t>({_pValue, _length}).value_or(0);
        break;
    case DatabaseFieldType::Int32:
        source = bBinary ? *reinterpret_cast<const int32_t *>(_pValue) : Util::StringTo<int32_t>({_pValue, _length}).value_or(0);
        break;
    case DatabaseFieldType::UInt64:
        source = bBinary ? *reinterpret_cast<const uint64_t *>(_pValue) : Util::StringTo<uint64_t>({_pValue, _length}).value_or(0);
        break;
    case DatabaseFieldType::Int64:
        source = bBinary ? *reinterpret_cast<const int64_t *>(_pValue) : Util::StringTo<int64_t>({_pValue, _length}).value_or(0);
        break;
    case DatabaseFieldType::Float:
        source = bBinary ? *reinterpret_cast<const float *>(_pValue) : Util::StringTo<float>({_pValue, _length}).value_or(0);
        break;
    case DatabaseFieldType::Double:
        source = bBinary ? *reinterpret_cast<const double *>(_pValue) : Util::StringTo<double>({_pValue, _length}).value_or(0);
        break;
    case DatabaseFieldType::Decimal:
    case DatabaseFieldType::Date:
    case DatabaseFieldType::Binary:
        break;
    }

    return std::visit(
        [](auto &&srcVal)
        {
            T result = static_cast<T>(srcVal);
            if (result != srcVal)
            {
                return std::make_pair(false, T{});
            }
            return std::make_pair(true, result);
        },
        source);
}

Field::Field()
    : _pValue(nullptr),
      _length(0),
      _meta(nullptr)
{
}

// DatabaseFieldType Field::GetType() const
// {
//     return _meta->fieldType;
// }

// bool Field::IsType(DatabaseFieldType type) const
// {
//     return _meta->fieldType == type;
// }

uint8_t Field::GetUInt8() const
{
    auto [bMatch, result] = CheckType<uint8_t>();
    if (!bMatch)
    {
        LogWrongGetter(__FUNCTION__);
    }

    return result;
}

int8_t Field::GetInt8() const
{
    auto [bMatch, result] = CheckType<int8_t>();
    if (!bMatch)
    {
        LogWrongGetter(__FUNCTION__);
    }

    return result;
}

uint16_t Field::GetUInt16() const
{
    auto [bMatch, result] = CheckType<uint16_t>();
    if (!bMatch)
    {
        LogWrongGetter(__FUNCTION__);
    }

    return result;
}

int16_t Field::GetInt16() const
{
    auto [bMatch, result] = CheckType<int16_t>();
    if (!bMatch)
    {
        LogWrongGetter(__FUNCTION__);
    }

    return result;
}

uint32_t Field::GetUInt32() const
{
    auto [bMatch, result] = CheckType<uint32_t>();
    if (!bMatch)
    {
        LogWrongGetter(__FUNCTION__);
    }

    return result;
}

int32_t Field::GetInt32() const
{
    auto [bMatch, result] = CheckType<int32_t>();
    if (!bMatch)
    {
        LogWrongGetter(__FUNCTION__);
    }

    return result;
}

uint64_t Field::GetUInt64() const
{
    auto [bMatch, result] = CheckType<uint64_t>();
    if (!bMatch)
    {
        LogWrongGetter(__FUNCTION__);
    }

    return result;
}

int64_t Field::GetInt64() const
{
    auto [bMatch, result] = CheckType<int64_t>();
    if (!bMatch)
    {
        LogWrongGetter(__FUNCTION__);
    }

    return result;
}
float Field::GetFloat() const
{
    auto [bMatch, result] = CheckType<float>();
    if (!bMatch)
    {
        LogWrongGetter(__FUNCTION__);
    }

    return result;
}

double Field::GetDouble() const
{
    auto [bMatch, result] = CheckType<double>();
    if (!bMatch)
    {
        LogWrongGetter(__FUNCTION__);
    }

    return result;
}

char const *Field::GetCString() const
{
    if (_meta->fieldType != DatabaseFieldType::Binary)
    {
        LogWrongGetter(__FUNCTION__);
        return nullptr;
    }

    return _pValue;
}

void Field::LogWrongGetter(std::string_view strGetter) const
{
    Assert(false, std::format("func：{}， 获取索引{}的类型：{} 字段：{}.{}({}.{})导致值被截断，请改用正确的方法调用",
                              strGetter, _meta->index,
                              _meta->typeName,
                              _meta->tableAlias,
                              _meta->alias,
                              _meta->tableName,
                              _meta->name));
}

void Field::SetValue(char const *newValue, uint32_t length)
{
    _pValue = newValue;
    _length = length;
}

void Field::SetMetadata(const QueryResultFieldMetadata *fieldMeta)
{
    Assert(fieldMeta != nullptr);
    _meta = fieldMeta;
}
