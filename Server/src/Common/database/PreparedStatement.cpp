/*************************************************************************
> File Name       : PreparedStatement.cpp
> Brief           : MySql预处理语句
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年08月03日  20时22分33秒
************************************************************************/
#include "pch.h"
#include "PreparedStatement.h"
#include "Common/include/Assert.h"

PreparedStatementBase::PreparedStatementBase(uint32_t preparedStatementIndex, uint8_t capacity)
    : _preparedStatementIndex(preparedStatementIndex), _statementData(capacity)
{
}

template <typename ValueType>
void PreparedStatementBase::SetValue(const uint8_t &index, SqlArgType type, ValueType &&value)
{
    Assert(index < _statementData.size());
    switch (type)
    {
    case SqlArgType::Uint8:
        _statementData[index].data.emplace<uint8_t>(value);
    case SqlArgType::Uint16:
        _statementData[index].data.emplace<uint16_t>(value);
    case SqlArgType::Uint32:
        _statementData[index].data.emplace<uint32_t>(value);
    case SqlArgType::Uint64:
        _statementData[index].data.emplace<uint64_t>(value);
    case SqlArgType::Int8:
        _statementData[index].data.emplace<int8_t>(value);
    case SqlArgType::Int16:
        _statementData[index].data.emplace<int16_t>(value);
    case SqlArgType::Int32:
        _statementData[index].data.emplace<int32_t>(value);
    case SqlArgType::Int64:
        _statementData[index].data.emplace<int64_t>(value);
    case SqlArgType::Float:
        _statementData[index].data.emplace<float>(value);
    case SqlArgType::Double:
        _statementData[index].data.emplace<double>(value);
    case SqlArgType::String:
        _statementData[index].data.emplace<std::string_view>(value);
    case SqlArgType::Binary:
        _statementData[index].data.emplace<std::vector<std::uint8_t>>(value);
    case SqlArgType::Null:
        _statementData[index].data.emplace<std::nullptr_t>(value);
        break;
    default:
        Assert(false, "预处理语句赋值不支持的类型");
    }
}
