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
#include "Common/threading/ProducerConsumerQueue.hpp"

enum class ConnectionType : uint8_t
{
    ASYNC      = 0x1,
    SYNC       = 0x2,
    ASYNC_SYNC = 0x1 | 0x2,
};

struct MySqlConnectionInfo
{
    std::string_view _user;
    std::string_view _password;
    std::string_view _database;
    std::string_view _host;
    std::string_view _port;
};

class SQLOperation;
class DatabaseWorker;

class MySqlConnection
{
public:
    explicit MySqlConnection(const MySqlConnectionInfo &connInfo);
    MySqlConnection(ProducerConsumerQueue<SQLOperation *> *queue, const MySqlConnectionInfo &connInfo);
    virtual ~MySqlConnection();
    MySqlConnection(const MySqlConnection &)            = delete;
    MySqlConnection &operator=(const MySqlConnection &) = delete;
    MySqlConnection(MySqlConnection &&)                 = delete;
    MySqlConnection &operator=(MySqlConnection &&)      = delete;

    /**
     * @brief 开启连接
     * @return uint32_t 错误码（ref mysql_errno）
     */
    virtual uint32_t Open();
    void             Close();
    bool             PrepareStatements();

    bool Execute(std::string_view sql);

protected:
    virtual void DoPrepareStatements() = 0;
    bool         HandleMysqlErrcode(uint32_t errcode, uint8_t tryReconnectTimes = 5);

protected:
    MySqlConnectionInfo _connectionInfo;
    bool                _reconnecting;
    bool                _prepareError;
    ConnectionType      _connectType;

private:
    MySqlHandle                           *_mysql;
    std::mutex                             _mutex;
    std::unique_ptr<DatabaseWorker>        _worker;
    ProducerConsumerQueue<SQLOperation *> *_queue;
};