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
#include "DatabaseEnv.h"
#include "Common/Threading/ProducerConsumerQueue.hpp"

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

class ISqlTask;
class DatabaseWorker;
class PreparedStatementBase;
class ResultSet;
class PreparedResultSet;
class MySqlPreparedStatement;
class TransactionBase;

class IMySqlConnection
{
    template <typename ConnectionType>
    friend class DatabaseWorkerPool;

public:
    explicit IMySqlConnection(const MySqlConnectionInfo &connInfo);
    IMySqlConnection(ProducerConsumerQueue<ISqlTask *> *queue, const MySqlConnectionInfo &connInfo);
    virtual ~IMySqlConnection();

    IMySqlConnection(const IMySqlConnection &)            = delete;
    IMySqlConnection &operator=(const IMySqlConnection &) = delete;
    IMySqlConnection(IMySqlConnection &&)                 = delete;
    IMySqlConnection &operator=(IMySqlConnection &&)      = delete;

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
    ResultSetPtr Query(std::string_view sql);

    /**
     * @brief 执行预处理语句查询
     *
     * @param stmt 预处理语句
     * @return PreparedResultSet* 查询结果
     */
    PreparedResultSetPtr Query(PreparedStatementBase *stmt);

    void     BeginTransaction();
    void     CommitTransaction();
    void     RollbackTransaction();
    uint32_t ExecuteTransaction(const std::shared_ptr<TransactionBase> &pTransaction);

    uint32_t GetLastError();

    void Ping() const;

protected:
    virtual void            DoPrepareStatements() = 0;
    void                    PrepareStatement(uint32_t index, std::string_view sql, ConnectionType connType);
    MySqlPreparedStatement *GetPrepareStatement(uint32_t index);

    bool TryLock();
    void UnLock();
    bool HandleMysqlErrcode(uint32_t errcode, uint8_t tryReconnectTimes = 5);

    bool Query(std::string_view sql,
               MySqlResult    *&pResult,
               MySqlField     *&pFields,
               uint64_t        &rowCount,
               uint32_t        &fieldCount);
    bool Query(PreparedStatementBase   *stmt,
               MySqlPreparedStatement *&pMySqlPreparedStmt,
               MySqlResult            *&pResult,
               uint64_t                &rowCount,
               uint32_t                &fieldCount);

protected:
    MySqlConnectionInfo                                  _connectionInfo;
    bool                                                 _reconnecting;
    bool                                                 _prepareError;
    ConnectionType                                       _connectType;
    std::vector<std::unique_ptr<MySqlPreparedStatement>> _stmts;

private:
    MySqlHandle                       *_mysql;
    std::mutex                         _mutex;
    std::unique_ptr<DatabaseWorker>    _worker;
    ProducerConsumerQueue<ISqlTask *> *_queue;
};