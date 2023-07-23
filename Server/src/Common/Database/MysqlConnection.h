/*************************************************************************
> File Name       : MysqlConnection.h
> Brief           : 数据库连接
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月22日  15时44分41秒
************************************************************************/
#pragma once

#include <string_view>
#include <mutex>
#include "MysqlTypeDef.h"

struct MySqlConnectionInfo
{
    std::string_view _user;
    std::string_view _password;
    std::string_view _database;
    std::string_view _host;
    std::string_view _port;
    std::string_view _ssl;
};

class MySqlConnection
{
public:
    explicit MySqlConnection(MySqlConnectionInfo &connInfo);
    virtual ~MySqlConnection();
    MySqlConnection(const MySqlConnection &)            = delete;
    MySqlConnection &operator=(const MySqlConnection &) = delete;
    MySqlConnection(MySqlConnection &&)                 = delete;
    MySqlConnection &operator=(MySqlConnection &&)      = delete;

    virtual uint32_t Open();
    void             Close();
    bool             PrepareStatements();

    bool Execute(std::string_view sql);
    Result

        private : MySqlHandle *_mysql;
    std::mutex                 _mutex;
};