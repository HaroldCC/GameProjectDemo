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
