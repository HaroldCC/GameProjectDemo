/*************************************************************************
> File Name       : SqlOperation.h
> Brief           : 数据库操作
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月28日  11时48分25秒
************************************************************************/
#pragma once

class MySqlConnection;

class SQLOperation
{
public:
    SQLOperation()          = default;
    virtual ~SQLOperation() = default;

    SQLOperation(const SQLOperation &)            = delete;
    SQLOperation(SQLOperation &&)                 = delete;
    SQLOperation &operator=(const SQLOperation &) = delete;
    SQLOperation &operator=(SQLOperation &&)      = delete;

    virtual bool Execute() = 0;
    virtual void SetConnection(MySqlConnection *sqlConn)
    {
        _sqlConn = sqlConn;
    }

private:
    MySqlConnection *_sqlConn = nullptr;
};