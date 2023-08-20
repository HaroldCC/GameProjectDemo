/*************************************************************************
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

Field::Field()
    : _pValue(nullptr),
      _length(0),
      _meta(nullptr)
{
}

DatabaseFieldType Field::GetType() const
{
}

bool Field::IsType(DatabaseFieldType type) const
{
    return _meta->fieldType == type;
}

uint8_t Field::GetUInt8() const
{
    if (!IsType(DatabaseFieldType::UInt8))
    {
        LogWrongGetter(__FUNCTION__);
    }

    return static_cast<uint8_t>(*_pValue);
}

int8_t Field::GetInt8() const
{
    if (!IsType(DatabaseFieldType::Int8))
    {
        LogWrongGetter(__FUNCTION__);
    }

    return static_cast<int8_t>(*_pValue);
}

uint16_t Field::GetUInt16() const
{
    if (!IsType(DatabaseFieldType::UInt16))
    {
        LogWrongGetter(__FUNCTION__);
    }

    return static_cast<uint16_t>(*_pValue);
}

int16_t Field::GetInt16() const
{
    if (!IsType(DatabaseFieldType::Int16))
    {
        LogWrongGetter(__FUNCTION__);
    }

    return static_cast<int16_t>(*_pValue);
}

uint32_t Field::GetUInt32() const
{
    if (!IsType(DatabaseFieldType::UInt32))
    {
        LogWrongGetter(__FUNCTION__);
    }

    return static_cast<uint32_t>(*_pValue);
}

int32_t Field::GetInt32() const
{
    if (!IsType(DatabaseFieldType::Int32))
    {
        LogWrongGetter(__FUNCTION__);
    }

    return static_cast<int32_t>(*_pValue);
}

uint64_t Field::GetUInt64() const
{
    if (!IsType(DatabaseFieldType::UInt64))
    {
        LogWrongGetter(__FUNCTION__);
    }

    return static_cast<uint64_t>(*_pValue);
}
int64_t Field::GetInt64() const
{
    if (!IsType(DatabaseFieldType::Int64))
    {
        LogWrongGetter(__FUNCTION__);
    }

    return static_cast<int64_t>(*_pValue);
}
float Field::GetFloat() const
{
    if (!IsType(DatabaseFieldType::Float))
    {
        LogWrongGetter(__FUNCTION__);
    }

    return static_cast<float>(*_pValue);
}
double Field::GetDouble() const
{
    if (!IsType(DatabaseFieldType::Double))
    {
        LogWrongGetter(__FUNCTION__);
    }

    return static_cast<double>(*_pValue);
}
char const *Field::GetCString() const
{
    if (!IsType(DatabaseFieldType::UInt8))
    {
        LogWrongGetter(__FUNCTION__);
    }

    return _pValue;
}

void Field::LogWrongGetter(const char *getter) const
{
    Assert(false, std::format("func：{}， 获取索引{}的类型：{} 字段：{}.{}({}.{})导致值被截断，请改用正确的方法调用",
                              getter, _meta->index,
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
    Assert(fieldMeta == nullptr);
    _meta = fieldMeta;
}
