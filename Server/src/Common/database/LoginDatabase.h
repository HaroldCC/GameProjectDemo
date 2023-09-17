/*************************************************************************
> File Name       : LoginDatabase.h
> Brief           : 登录数据库连接
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年08月07日  11时56分49秒
************************************************************************/
#pragma once

#include <array>
#include <vector>
#include <string>
#include <cstdint>
#include <unordered_map>

#include "DatabaseWorkerPool.h"
#include "MysqlConnection.h"

enum LoginDatabaseEnum : uint32_t
{
    LOGIN_SEL_ACCOUNT_BY_EMAIL,
    LOGIN_INS_ACCOUNT,

    LoginDatabaseEnum_MAX,
};

struct SqlStmtData
{
    std::string_view        sql;                // sql语句串
    std::vector<SqlArgType> argTypes;           // 语句串中变量的类型（从左到右依次）
    ConnectionType          joinConnectionType; // 添加至的连接类型（同步or异步）
};

// clang-format off
const std::unordered_map<LoginDatabaseEnum, SqlStmtData> g_LoginDatabaseStmtMap =
{
    {LoginDatabaseEnum::LOGIN_SEL_ACCOUNT_BY_EMAIL,
        {"select id, username from account where email= ?",
            {SqlArgType::String},
            ConnectionType::SYNC
        }
    },
    {
        LoginDatabaseEnum::LOGIN_INS_ACCOUNT,
        {
            "insert into account (email, password, joinDate) values(?, ?, NOW())",
            {SqlArgType::String, SqlArgType::String},
            ConnectionType::ASYNC
        }
    },
};
// clang-format on

class LoginDatabaseConnection : public IMySqlConnection
{
public:
    using IMySqlConnection::IMySqlConnection;

    void DoPrepareStatements() override;
};

using LoginDatabasePreparedStatement = PreparedStatement<LoginDatabaseConnection>;

extern DatabaseWorkerPool<LoginDatabaseConnection> g_LoginDatabase;