/*************************************************************************
> File Name       : MySqlPreparedStatement.cpp
> Brief           : 数据库预处理语句
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年08月21日  15时13分59秒
************************************************************************/
#include "pch.h"
#include <type_traits>
#include <variant>
#include "MySqlTypeHack.h"

#include "MySqlPreparedStatement.h"
#include "PreparedStatement.h"
#include "Common/include/Log.hpp"
#include "Common/include/Assert.h"

MySqlPreparedStatement::MySqlPreparedStatement(MySqlStmt *pMySqlStmt, std::string_view sqlString)
    : _pMySqlStmt(pMySqlStmt),
      _pStmt(nullptr),
      _paramCount(mysql_stmt_param_count(pMySqlStmt)),
      _paramAssignFlag(_paramCount),
      _pBind(new MySqlBind[_paramCount]),
      _sqlString(sqlString)
{
    _paramAssignFlag.assign(_paramCount, false);

    memset(_pBind, 0, sizeof(MySqlBind) * _paramCount);

    MySqlBool bFlag = true;
    // 设置在执行mysql_stmt_store_result时计算MYSQL_FIELD->max_length的值
    mysql_stmt_attr_set(_pMySqlStmt, STMT_ATTR_UPDATE_MAX_LENGTH, &bFlag);
}

MySqlPreparedStatement::~MySqlPreparedStatement()
{
    ClearParameters();
    if (_pMySqlStmt->bind_result_done)
    {
        delete[] _pMySqlStmt->bind->length;
        delete[] _pMySqlStmt->bind->is_null;
    }

    mysql_stmt_close(_pMySqlStmt);
    delete[] _pBind;
    _pBind = nullptr;
}

void MySqlPreparedStatement::BindParameters(PreparedStatementBase *pStmt)
{
    _pStmt = pStmt;

    uint8_t pos = 0;
    for (const PreparedStatementData &data : _pStmt->GetParameters())
    {
        std::visit([&](auto &&param)
                   { SetParameter(pos, param); },
                   data.data);
        ++pos;
    }

#if defined(DEBUG) || defined(_DEBUG)
    if (pos < _paramCount)
    {
        Log::Warn("预处理语句id：{} 未成功绑定所有参数", _pStmt->GetIndex());
    }
#endif
}

template <typename T>
void MySqlPreparedStatement::SetParameter(uint8_t index, T &&value)
{
    AssertValidIndex(index);
    _paramAssignFlag[index] = true;
    MySqlBind *pParam       = &_pBind[index];
    uint32_t   length       = sizeof(T);
    pParam->buffer_type     = MysqlType<std::decay_t<T>>::value;
    delete[] static_cast<char *>(pParam->buffer);
    pParam->buffer        = new char[length];
    pParam->buffer_length = 0;
    pParam->is_null_value = false;
    pParam->length        = nullptr;
    pParam->is_unsigned   = std::is_signed_v<T>;

    memcpy(pParam->buffer, &value, length);
}

void MySqlPreparedStatement::SetParameter(uint8_t index, std::nullptr_t)
{
    AssertValidIndex(index);
    _paramAssignFlag[index] = true;
    MySqlBind *pParam       = &_pBind[index];
    pParam->buffer_type     = MYSQL_TYPE_NULL;
    delete[] static_cast<char *>(pParam->buffer);
    pParam->buffer        = nullptr;
    pParam->buffer_length = 0;
    pParam->is_null_value = true;
    delete pParam->length;
    pParam->length = nullptr;
}

void MySqlPreparedStatement::SetParameter(uint8_t index, bool value)
{
    SetParameter(index, uint8_t(value ? 1 : 0));
}

void MySqlPreparedStatement::SetParameter(uint8_t index, const std::string &str)
{
    AssertValidIndex(index);
    _paramAssignFlag[index] = true;
    MySqlBind *pParam       = &_pBind[index];
    uint32_t   length       = (uint32_t)str.size();
    pParam->buffer_type     = MYSQL_TYPE_VAR_STRING;
    delete[] static_cast<char *>(pParam->buffer);
    pParam->buffer        = new char[length];
    pParam->buffer_length = length;
    pParam->is_null_value = false;
    delete pParam->length;
    pParam->length = new unsigned long(length);

    memcpy(pParam->buffer, str.data(), length);
}

void MySqlPreparedStatement::SetParameter(uint8_t index, const std::vector<uint8_t> &value)
{
    AssertValidIndex(index);
    _paramAssignFlag[index] = true;
    MySqlBind *pParam       = &_pBind[index];
    uint32_t   length       = (uint32_t)value.size();
    pParam->buffer_type     = MYSQL_TYPE_BLOB;
    delete[] static_cast<char *>(pParam->buffer);
    pParam->buffer        = new char[length];
    pParam->buffer_length = length;
    pParam->is_null_value = false;
    delete pParam->length;
    pParam->length = new unsigned long(length);

    memcpy(pParam->buffer, value.data(), length);
}

void MySqlPreparedStatement::AssertValidIndex(uint8_t index)
{
    Assert(index < _paramCount,
           std::format("预处理语句id：{} 绑定第{}个参数错误，该语句共需{}个参数", _pStmt->GetIndex(), index, _paramCount));

    if (_paramAssignFlag[index])
    {
        Log::Error("预处理语句id：{} 第{}个参数已经绑定", _pStmt->GetIndex(), index);
    }
}

void MySqlPreparedStatement::ClearParameters()
{
    for (uint32_t i = 0; i < _paramCount; ++i)
    {
        delete[] _pBind[i].length;
        _pBind[i].length = nullptr;
        delete[] static_cast<char *>(_pBind[i].buffer);
        _pBind[i].buffer    = nullptr;
        _paramAssignFlag[i] = false;
    }
}

std::string MySqlPreparedStatement::GetSqlString() const
{
    std::string sqlString(_sqlString);
    size_t      pos = 0;
    for (const PreparedStatementData &data : _pStmt->GetParameters())
    {
        pos = sqlString.find('?', pos);

        std::string replaceStr = std::visit([&](auto &&data)
                                            { return PreparedStatementData::ToString(data); },
                                            data.data);
        sqlString.replace(pos, 1, replaceStr);
        pos += replaceStr.length();
    }

    return sqlString;
}
