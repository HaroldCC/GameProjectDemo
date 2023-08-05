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

PrepareStatementBase::PrepareStatementBase(uint32_t prepareStatementIndex, uint8_t capacity)
    : _prepareStatementIndex(prepareStatementIndex), _statementData(capacity)
{
}
