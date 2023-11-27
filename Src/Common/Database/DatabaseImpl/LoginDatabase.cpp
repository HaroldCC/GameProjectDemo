/*************************************************************************
> File Name       : LoginDatabase.cpp
> Brief           : 登录数据库连接
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年09月15日  16时14分57秒
************************************************************************/
#include "LoginDatabase.h"
#include "Common/Util/Util.hpp"
#include "Common/Database/MySqlPreparedStatement.h"
#include "Common/Database/QueryResult.h"

DatabaseWorkerPool<LoginDatabaseConnection> g_LoginDatabase;

void LoginDatabaseConnection::DoPrepareStatements()
{
    if (!_reconnecting)
    {
        _stmts.resize(Util::ToUnderlying(LoginDatabaseEnum::LoginDatabaseEnum_MAX));
    }

    for (auto &&item : g_LoginDatabaseStmtMap)
    {
        PrepareStatement(item.first, item.second.sql, item.second.joinConnectionType);
    }
}