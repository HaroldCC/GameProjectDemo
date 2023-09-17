/*************************************************************************
> File Name       : LoginDatabase.cpp
> Brief           : 登录数据库连接
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年09月15日  16时14分57秒
************************************************************************/

#include "pch.h"
#include "LoginDatabase.h"

DatabaseWorkerPool<LoginDatabaseConnection> g_LoginDatabase;

void LoginDatabaseConnection::DoPrepareStatements()
{
    for (auto &&item : g_LoginDatabaseStmtMap)
    {
        PrepareStatement(item.first, item.second.sql, item.second.joinConnectionType);
    }
}