/*************************************************************************
> File Name       : MySqlPreparedStatement.h
> Brief           : 数据库预处理语句
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年08月21日  14时49分51秒
************************************************************************/
#pragma once

#include <string_view>
#include <vector>
#include "MysqlTypeDef.h"

class PreparedStatementBase;

class MySqlPreparedStatement
{
    friend class IMySqlConnection;
    friend class PreparedStatementBase;

public:
    MySqlPreparedStatement(MySqlStmt *pMySqlStmt, std::string_view sqlString);
    ~MySqlPreparedStatement();

    MySqlPreparedStatement(const MySqlPreparedStatement &)            = delete;
    MySqlPreparedStatement &operator=(const MySqlPreparedStatement &) = delete;
    MySqlPreparedStatement(MySqlPreparedStatement &&)                 = delete;
    MySqlPreparedStatement &operator=(MySqlPreparedStatement &&)      = delete;

    void BindParameters(PreparedStatementBase *pStmt);

    [[nodiscard]] uint32_t GetParameterCount() const
    {
        return _paramCount;
    }

private:
    template <typename T>
    void SetParameter(uint8_t index, T &&value);
    void SetParameter(uint8_t index, std::nullptr_t);
    void SetParameter(uint8_t index, bool value);
    void SetParameter(uint8_t index, std::string_view str);
    void SetParameter(uint8_t index, const std::vector<uint8_t> &value);

    void AssertValidIndex(uint8_t index);

    [[nodiscard]] MySqlStmt *GetMySqlStmt() const
    {
        return _pMySqlStmt;
    }

    [[nodiscard]] MySqlBind *GetMySqlBind() const
    {
        return _pBind;
    }

    void ClearParameters();

    [[nodiscard]] std::string GetSqlString() const;

private:
    MySqlStmt             *_pMySqlStmt;
    PreparedStatementBase *_pStmt;
    uint32_t               _paramCount;
    std::vector<bool>      _paramAssignFlag;
    MySqlBind             *_pBind;
    std::string_view       _sqlString;
};