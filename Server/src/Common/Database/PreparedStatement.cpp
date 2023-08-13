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

PreparedStatementBase::PreparedStatementBase(uint32_t preparedStatementIndex, uint8_t capacity)
    : _preparedStatementIndex(preparedStatementIndex), _statementData(capacity)
{
}

template <typename ValueType>
void PreparedStatementBase::SetValue(const uint8_t &index, const ValueType &value)
{
    assert(index < _statementData.size());
    _statementData[index]._data = value;
}
