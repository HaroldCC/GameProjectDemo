/*************************************************************************
> File Name       : LoginDatabase.h
> Brief           : 登录数据库
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

enum LoginDatabaseEnum : uint32_t
{
    INSERT_CHARACTER_INFO,
    INSERT_ACCOUNT_INFO,

    LoginDatabaseEnum_MAX,
};

enum class SQL_ARG_TYPE : uint32_t
{
    String,
    Int,
    Bool,
    Double,
};

struct SQL_STMT_DATA
{
    std::string_view          sql;
    std::vector<SQL_ARG_TYPE> argTypes;
};

const std::unordered_map<LoginDatabaseEnum, SQL_STMT_DATA> LOGIN_DATABASE = {
    {LoginDatabaseEnum::INSERT_CHARACTER_INFO, {"insert into Character (name, sex, power) values (?, ?, ?)", {SQL_ARG_TYPE::String, SQL_ARG_TYPE::Bool, SQL_ARG_TYPE::Double}}},
    {LoginDatabaseEnum::INSERT_ACCOUNT_INFO, {"insert into Account (account, password) values (?, ?)", {SQL_ARG_TYPE::String, SQL_ARG_TYPE::String}}},
};

const std::unordered_map<LoginDatabaseEnum, SQL_STMT_DATA> LOGIN_DATABASE = {
    {LoginDatabaseEnum::INSERT_CHARACTER_INFO, {"insert into Character (name, sex, power) values (?, ?, ?)", {SQL_ARG_TYPE::String, SQL_ARG_TYPE::Bool, SQL_ARG_TYPE::Double}}},
    {LoginDatabaseEnum::INSERT_ACCOUNT_INFO, {"insert into Account (account, password) values (?, ?)", {SQL_ARG_TYPE::String, SQL_ARG_TYPE::String}}},
    {LoginDatabaseEnum::INSERT_ACCOUNT_INFO3, {"insert into Account (account, password) values (?, ?)", {SQL_ARG_TYPE::String, SQL_ARG_TYPE::String}}},
    {LoginDatabaseEnum::INSERT_ACCOUNT_INFO4, {"insert into Account (account, password) values (?, ?)", {SQL_ARG_TYPE::String, SQL_ARG_TYPE::String}}},
    {LoginDatabaseEnum::INSERT_ACCOUNT_INFO5, {"insert into Account (account, password) values (?, ?)", {SQL_ARG_TYPE::String, SQL_ARG_TYPE::String}}},
    {LoginDatabaseEnum::INSERT_ACCOUNT_INFO6, {"insert into Account (account, password) values (?, ?)", {SQL_ARG_TYPE::String, SQL_ARG_TYPE::String}}},
    {LoginDatabaseEnum::INSERT_ACCOUNT_INFO7, {"insert into Account (account, password) values (?, ?)", {SQL_ARG_TYPE::String, SQL_ARG_TYPE::String}}},
    {LoginDatabaseEnum::INSERT_ACCOUNT_INFO8, {"insert into Account (account, password) values (?, ?)", {SQL_ARG_TYPE::String, SQL_ARG_TYPE::String}}},
    {LoginDatabaseEnum::INSERT_ACCOUNT_INFO9, {"insert into Account (account, password) values (?, ?)", {SQL_ARG_TYPE::String, SQL_ARG_TYPE::String}}},
    {LoginDatabaseEnum::INSERT_ACCOUNT_INFO10, {"insert into Account (account, password) values (?, ?)", {SQL_ARG_TYPE::String, SQL_ARG_TYPE::String}}},
    {LoginDatabaseEnum::INSERT_ACCOUNT_INFO11, {"insert into Account (account, password) values (?, ?)", {SQL_ARG_TYPE::String, SQL_ARG_TYPE::String}}},
    {LoginDatabaseEnum::INSERT_ACCOUNT_INFO12, {"insert into Account (account, password) values (?, ?)", {SQL_ARG_TYPE::String, SQL_ARG_TYPE::String}}},
    {LoginDatabaseEnum::INSERT_ACCOUNT_INFO13, {"insert into Account (account, password) values (?, ?)", {SQL_ARG_TYPE::String, SQL_ARG_TYPE::String}}},
    {LoginDatabaseEnum::INSERT_ACCOUNT_INFO14, {"insert into Account (account, password) values (?, ?)", {SQL_ARG_TYPE::String, SQL_ARG_TYPE::String}}},
    {LoginDatabaseEnum::INSERT_ACCOUNT_INFO15, {"insert into Account (account, password) values (?, ?)", {SQL_ARG_TYPE::String, SQL_ARG_TYPE::String}}},
    {LoginDatabaseEnum::INSERT_ACCOUNT_INFO16, {"insert into Account (account, password) values (?, ?)", {SQL_ARG_TYPE::String, SQL_ARG_TYPE::String}}},
    {LoginDatabaseEnum::INSERT_ACCOUNT_INFO17, {"insert into Account (account, password) values (?, ?)", {SQL_ARG_TYPE::String, SQL_ARG_TYPE::String}}},
    {LoginDatabaseEnum::INSERT_ACCOUNT_INFO18, {"insert into Account (account, password) values (?, ?)", {SQL_ARG_TYPE::String, SQL_ARG_TYPE::String}}},
    {LoginDatabaseEnum::INSERT_ACCOUNT_INFO19, {"insert into Account (account, password) values (?, ?)", {SQL_ARG_TYPE::String, SQL_ARG_TYPE::String}}},
    {LoginDatabaseEnum::INSERT_ACCOUNT_INFO20, {"insert into Account (account, password) values (?, ?)", {SQL_ARG_TYPE::String, SQL_ARG_TYPE::String}}},
    {LoginDatabaseEnum::INSERT_ACCOUNT_INFO21, {"insert into Account (account, password) values (?, ?)", {SQL_ARG_TYPE::String, SQL_ARG_TYPE::String}}},
    {LoginDatabaseEnum::INSERT_ACCOUNT_INFO22, {"insert into Account (account, password) values (?, ?)", {SQL_ARG_TYPE::String, SQL_ARG_TYPE::String}}},
    {LoginDatabaseEnum::INSERT_ACCOUNT_INFO23, {"insert into Account (account, password) values (?, ?)", {SQL_ARG_TYPE::String, SQL_ARG_TYPE::String}}},
    {LoginDatabaseEnum::INSERT_ACCOUNT_INFO24, {"insert into Account (account, password) values (?, ?)", {SQL_ARG_TYPE::String, SQL_ARG_TYPE::String}}},
    {LoginDatabaseEnum::INSERT_ACCOUNT_INFO25, {"insert into Account (account, password) values (?, ?)", {SQL_ARG_TYPE::String, SQL_ARG_TYPE::String}}},
    {LoginDatabaseEnum::INSERT_ACCOUNT_INFO26, {"insert into Account (account, password) values (?, ?)", {SQL_ARG_TYPE::String, SQL_ARG_TYPE::String}}},
    {LoginDatabaseEnum::INSERT_ACCOUNT_INFO27, {"insert into Account (account, password) values (?, ?)", {SQL_ARG_TYPE::String, SQL_ARG_TYPE::String}}},
    {LoginDatabaseEnum::INSERT_ACCOUNT_INFO28, {"insert into Account (account, password) values (?, ?)", {SQL_ARG_TYPE::String, SQL_ARG_TYPE::String}}},
    {LoginDatabaseEnum::INSERT_ACCOUNT_INFO29, {"insert into Account (account, password) values (?, ?)", {SQL_ARG_TYPE::String, SQL_ARG_TYPE::String}}},
    {LoginDatabaseEnum::INSERT_ACCOUNT_INFO30, {"insert into Account (account, password) values (?, ?)", {SQL_ARG_TYPE::String, SQL_ARG_TYPE::String}}},
    {LoginDatabaseEnum::INSERT_ACCOUNT_INFO31, {"insert into Account (account, password) values (?, ?)", {SQL_ARG_TYPE::String, SQL_ARG_TYPE::String}}},
    {LoginDatabaseEnum::INSERT_ACCOUNT_INFO32, {"insert into Account (account, password) values (?, ?)", {SQL_ARG_TYPE::String, SQL_ARG_TYPE::String}}},

};
