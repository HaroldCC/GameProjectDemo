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
class PreparedStatementBase;
class ResultSet;
class PreparedResultSet;

class MySqlConnection
{
    template <typename ConnectionType>
    friend class DatabaseWorkerPool;

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

    /**
     * @brief 执行sql语句，不需要结果
     *
     * @param sql sql语句
     * @return bool 执行是否成功
     */
    bool Execute(std::string_view sql);

    /**
     * @brief 指定预处理语句，不需要结果
     *
     * @param stmt 预处理语句
     * @return bool 执行是否成功
     */
    bool Execute(PreparedStatementBase *stmt);

    /**
     * @brief 执行sql语句查询
     *
     * @param sql sql语句
     * @return ResultSet* 查询结果
     */
    ResultSet *Query(std::string_view sql);

    /**
     * @brief 执行预处理语句查询
     *
     * @param stmt 预处理语句
     * @return PreparedResultSet* 查询结果
     */
    PreparedResultSet *Query(PreparedStatementBase *stmt);

    void BeginTransaction();
    void CommitTransaction();
    void RollbackTransaction();

    uint32_t GetLastError();

protected:
    virtual void DoPrepareStatements() = 0;

    bool TryLock();
    void UnLock();
    bool HandleMysqlErrcode(uint32_t errcode, uint8_t tryReconnectTimes = 5);
    bool Query(std::string_view sql, MySqlResult *&pResult, MySqlField *&pFields,
               uint64_t *pRowCount, uint32_t *pFieldCount);
    bool Query(PreparedStatementBase *stmt, MySqlResult *&pResult, MySqlField *&pFields,
               uint64_t *pRowCount, uint32_t *pFieldCount);

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