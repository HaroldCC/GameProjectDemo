/*************************************************************************
> File Name       : MySqlConnection.cpp
> Brief           : 数据库连接
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月27日  15时37分16秒
************************************************************************/
#include "pch.h"
#include "MysqlConnection.h"
#include "Common/include/Log.hpp"
#include "Common/include/Performance.hpp"
#include "mysqld_error.h"
#include "DatabaseWorker.h"
#include "PreparedStatement.h"
#include "MySqlPreparedStatement.h"
#include "Common/include/Assert.h"
#include "QueryResult.h"
#include "Transaction.h"
#include "MySqlTypeHack.h"

using namespace std::literals::chrono_literals;

IMySqlConnection::IMySqlConnection(const MySqlConnectionInfo &connInfo)
    : _connectionInfo(connInfo),
      _reconnecting(false),
      _prepareError(false),
      _connectType(ConnectionType::SYNC),
      _mysql(nullptr),
      _queue(nullptr)
{
}

IMySqlConnection::IMySqlConnection(ProducerConsumerQueue<ISqlTask *> *queue, const MySqlConnectionInfo &connInfo)
    : _connectionInfo(connInfo),
      _reconnecting(false),
      _prepareError(false),
      _connectType(ConnectionType::ASYNC),
      _mysql(nullptr),
      _queue(queue)
{
    _worker = std::make_unique<DatabaseWorker>(queue, this);
}

IMySqlConnection::~IMySqlConnection()
{
    Close();
}

/**
 * @brief 开启连接
 *
 * @return uint32_t 错误码（ref mysql_errno）
 */
uint32_t IMySqlConnection::Open()
{
    _mysql = mysql_init(nullptr);
    if (nullptr == _mysql)
    {
        Log::Error("初始化数据库：{}失败", _connectionInfo._database);
        return CR_UNKNOWN_ERROR;
    }

    // 设置options
    mysql_options(_mysql, MYSQL_SET_CHARSET_NAME, "utf8mb4");

    uint32_t port = std::atoi(_connectionInfo._port.data());
    _mysql        = mysql_real_connect(_mysql, _connectionInfo._host.data(),
                                       _connectionInfo._user.data(),
                                       _connectionInfo._password.data(),
                                       _connectionInfo._database.data(),
                                       port, nullptr, 0);
    if (nullptr == _mysql)
    {
        uint32_t errcode = mysql_errno(_mysql);
        Log::Error("连接数据库失败:{} [host:{}, user:{}, password:{}, database:{}, port:{}]", mysql_error(_mysql),
                   _connectionInfo._host, _connectionInfo._user,
                   _connectionInfo._password, _connectionInfo._database, port);
        mysql_close(_mysql);
        return errcode;
    }

    if (!_reconnecting)
    {
        Log::Info("Mysql client library:{}", mysql_get_client_info());
        Log::Info("Mysql server version:{}", mysql_get_server_info(_mysql));
    }

    mysql_autocommit(_mysql, true);
    mysql_set_character_set(_mysql, "utf8mb4");
    return 0;
}

void IMySqlConnection::Close()
{
    _worker.reset();
    _stmts.clear();

    if (nullptr != _mysql)
    {
        mysql_close(_mysql);
        _mysql = nullptr;
    }
}

bool IMySqlConnection::PrepareStatements()
{
    DoPrepareStatements();
    return !_prepareError;
}

bool IMySqlConnection::Execute(std::string_view sql)
{
    if (nullptr == _mysql)
    {
        return false;
    }

    {
        Util::Timer timer;
        if (0 != mysql_query(_mysql, sql.data()))
        {
            uint32_t errcode = mysql_errno(_mysql);
            Log::Info("执行sql脚本：SQL:{}", sql);
            Log::Error("执行sql脚本出错：[{}]{}", errcode, mysql_error(_mysql));

            // 处理mysql错误，如果返回true，表示成功处理，继续尝试执行
            if (HandleMysqlErrcode(errcode))
            {
                return Execute(sql);
            }

            return false;
        }

        Log::Debug("执行脚本成功，耗时：{} ms SQL:{}", timer.ElapsedMillisec(), sql);
    }

    return true;
}

/**
 * @brief 指定预处理语句，不需要结果
 *
 * @param stmt 预处理语句
 * @return bool 执行是否成功
 */
bool IMySqlConnection::Execute(PreparedStatementBase *pStmt)
{
    if (nullptr == _mysql)
    {
        return false;
    }

    uint32_t                index              = pStmt->GetIndex();
    MySqlPreparedStatement *pMySqlPreparedStmt = GetPrepareStatement(index);
    Assert(nullptr != pMySqlPreparedStmt, std::format("索引为{}的预处理语句未成功加载，请排查原因", index));

    pMySqlPreparedStmt->BindParameters(pStmt);
    MySqlStmt *pMySqlStmt = pMySqlPreparedStmt->GetMySqlStmt();
    MySqlBind *pMysqlBind = pMySqlPreparedStmt->GetMySqlBind();

    {
        PERFORMANCE_SCOPE(std::format("执行Sql语句：[{}]", pMySqlPreparedStmt->GetSqlString()));

        if (mysql_stmt_bind_param(pMySqlStmt, pMysqlBind))
        {
            uint32_t errcode = mysql_errno(_mysql);
            Log::Debug("sql:[{}], 绑定参数错误：{}:{}",
                       pMySqlPreparedStmt->GetSqlString(), errcode, mysql_stmt_errno(pMySqlStmt));

            if (HandleMysqlErrcode(errcode))
            {
                // 处理错误成功，则继续执行
                return Execute(pStmt);
            }

            pMySqlPreparedStmt->ClearParameters();
            return false;
        }

        if (0 != mysql_stmt_execute(pMySqlStmt))
        {
            uint32_t errcode = mysql_errno(_mysql);
            Log::Error("执行Sql语句：[{}] 出错：{}:{}",
                       pMySqlPreparedStmt->GetSqlString(), errcode, mysql_stmt_error(pMySqlStmt));

            if (HandleMysqlErrcode(errcode))
            {
                // 处理错误成功，继续执行
                return Execute(pStmt);
            }

            pMySqlPreparedStmt->ClearParameters();
            return false;
        }
    }

    pMySqlPreparedStmt->ClearParameters();
    return true;
}

uint32_t IMySqlConnection::GetLastError()
{
    return mysql_errno(_mysql);
}

void IMySqlConnection::PrepareStatement(uint32_t index, std::string_view sql, ConnectionType connType)
{
    // 判断要加载的预处理语句是否与当前连接的类型一致
    if (!(Util::ToUnderlying(_connectType) & Util::ToUnderlying(connType)))
    {
        _stmts[index].reset();
        return;
    }

    MySqlStmt *pStmt = mysql_stmt_init(_mysql);
    if (nullptr == pStmt)
    {
        Log::Error("初始化预处理语句：{}:{} 错误：{}", index, sql, mysql_error(_mysql));
        _prepareError = true;
    }
    else
    {
        if (0 != mysql_stmt_prepare(pStmt, sql.data(), static_cast<uint32_t>(sql.size())))
        {
            Log::Error("处理预处理语句：{}:{} 错误：{}", index, sql, mysql_stmt_error(pStmt));
            mysql_stmt_close(pStmt);
            _prepareError = true;
        }
        else
        {
            _stmts[index] = std::make_unique<MySqlPreparedStatement>(pStmt, sql);
        }
    }
}

MySqlPreparedStatement *IMySqlConnection::GetPrepareStatement(uint32_t index)
{
    Assert(index < _stmts.size(),
           std::format("尝试获取非法的预处理语句，索引：{} (最大索引：{}) 数据库：{}, 连接方式：{}", index, _stmts.size(),
                       _connectionInfo._database,
                       ((uint8_t)_connectType & (uint8_t)ConnectionType::ASYNC) ? "异步" : "同步"));
    MySqlPreparedStatement *pMySqlPreparedStmt = _stmts[index].get();
    if (nullptr == pMySqlPreparedStmt)
    {
        Log::Error("获取数据库：{} 预处理语句索引 {} 失败，连接方式：{}",
                   _connectionInfo._database, index,
                   (Util::ToUnderlying(_connectType) &
                    Util::ToUnderlying(ConnectionType::ASYNC))
                       ? "异步"
                       : "同步");
    }

    return pMySqlPreparedStmt;
}

ResultSetPtr IMySqlConnection::Query(std::string_view sql)
{
    if (sql.empty())
    {
        return nullptr;
    }

    MySqlResult *pMySqlResult = nullptr;
    MySqlField  *pFields      = nullptr;
    uint64_t     rowCount     = 0;
    uint32_t     fieldCount   = 0;
    if (!Query(sql, pMySqlResult, pFields, rowCount, fieldCount))
    {
        return nullptr;
    }

    return MakeResultSetPtr(pMySqlResult, pFields, rowCount, fieldCount);
}

PreparedResultSetPtr IMySqlConnection::Query(PreparedStatementBase *pStmt)
{
    MySqlPreparedStatement *pMySqlPreparedStmt = nullptr;
    MySqlResult            *pMySqlResult       = nullptr;
    uint64_t                rowCount           = 0;
    uint32_t                fieldCount         = 0;
    if (!Query(pStmt, pMySqlPreparedStmt, pMySqlResult, rowCount, fieldCount))
    {
        return nullptr;
    }

    if (mysql_more_results(_mysql))
    {
        mysql_next_result(_mysql);
    }

    return MakePreparedResultSetPtr(pMySqlPreparedStmt->GetMySqlStmt(), pMySqlResult, rowCount, fieldCount);
}

void IMySqlConnection::BeginTransaction()
{
    Execute("START TRANSACTION");
}

void IMySqlConnection::CommitTransaction()
{
    Execute("COMMIT");
}

void IMySqlConnection::RollbackTransaction()
{
    Execute("ROLLBACK");
}

uint32_t IMySqlConnection::ExecuteTransaction(const std::shared_ptr<TransactionBase> &pTransaction)
{
    const std::vector<TransactionBase::SqlElementData> &queries = pTransaction->_queries;
    if (queries.empty())
    {
        return -1;
    }

    BeginTransaction();

    for (const auto &query : queries)
    {
        if (std::holds_alternative<PreparedStatementBase *>(query))
        {
            PreparedStatementBase *pStmt = std::get<PreparedStatementBase *>(query);
            Assert(nullptr != pStmt);
            if (!Execute(pStmt))
            {
                Log::Warn("执行事务失败，即将回滚，事务队列数量{}", queries.size());
                RollbackTransaction();

                return GetLastError();
            }
        }
        else
        {
            std::string_view sql = std::get<std::string_view>(query);
            if (!Execute(sql))
            {
                Log::Warn("执行事务失败，即将回滚，事务队列数量{}", queries.size());
                RollbackTransaction();
                return GetLastError();
            }
        }
    }

    CommitTransaction();

    return 0;
}

bool IMySqlConnection::TryLock()
{
    return _mutex.try_lock();
}

void IMySqlConnection::UnLock()
{
    _mutex.unlock();
}

bool IMySqlConnection::HandleMysqlErrcode(uint32_t errcode, uint8_t tryReconnectTimes /* = 5 */)
{
    switch (errcode)
    {
    case CR_SERVER_GONE_ERROR:
    case CR_SERVER_LOST:
    case CR_SERVER_LOST_EXTENDED:
    {
        if (nullptr != _mysql)
        {
            Log::Error("Mysql server 连接丢失");
            mysql_close(_mysql);
            _mysql = nullptr;
        }
    }
    case CR_CONN_HOST_ERROR:
    {
        Log::Info("Reconnecting Mysql server......");
        _reconnecting          = true;
        const uint32_t errcode = Open();
        if (0 != errcode)
        {
            if (!this->PrepareStatements())
            {
                Log::Critical("处理预处理sql语句失败，数据库连接即将断开!!!");
                std::this_thread::sleep_for(10s);
                std::abort();
            }

            Log::Info("重连数据库成功：{} @{}:{} {}", _connectionInfo._database,
                      _connectionInfo._host, _connectionInfo._port,
                      ((uint8_t)_connectType & (uint8_t)ConnectionType::ASYNC) ? "异步方式" : "同步方式");
            _reconnecting = false;
            return true;
        }

        if ((--tryReconnectTimes) == 0)
        {
            Log::Critical("多次尝试重连数据库失败，服务中断！！！");
            std::this_thread::sleep_for(10s);
            std::abort();
        }
        else
        {
            std::this_thread::sleep_for(3s);
            return HandleMysqlErrcode(_prepareError, tryReconnectTimes);
        }
    }
    case ER_LOCK_DEADLOCK:
    // Implemented in TransactionTask::Execute and DatabaseWorkerPool<T>::DirectCommitTransaction
    // Query related errors - skip query
    case ER_WRONG_VALUE_COUNT:
    case ER_DUP_ENTRY:
        return false;

    // Outdated table or database structure - terminate core
    case ER_BAD_FIELD_ERROR:
    case ER_NO_SUCH_TABLE:
        Log::Error("数据库未找到对应的表");
        std::this_thread::sleep_for(std::chrono::seconds(10));
        std::abort();
        return false;
    case ER_PARSE_ERROR:
        Log::Error("数据库脚本出错，请检查语法");
        std::this_thread::sleep_for(std::chrono::seconds(10));
        std::abort();
        return false;
    default:
    {
        Log::Error("未处理的Mysql 错误，错误码：{}", errcode);
        return false;
    }
    }

    return false;
}

bool IMySqlConnection::Query(std::string_view sql, MySqlResult *&pResult, MySqlField *&pFields,
                             uint64_t &rowCount, uint32_t &fieldCount)
{
    if (nullptr == _mysql)
    {
        return false;
    }

    {
        PERFORMANCE_SCOPE(std::format("执行Sql语句[{}]", sql));

        if (mysql_query(_mysql, sql.data()))
        {
            uint32_t errcode = mysql_errno(_mysql);
            Log::Error("执行sql语句出错：{}:{}", errcode, mysql_error(_mysql));
            if (HandleMysqlErrcode(errcode))
            {
                // 成功处理错误，再次执行
                return Query(sql, pResult, pFields, rowCount, fieldCount);
            }

            return false;
        }
    }

    pResult    = mysql_store_result(_mysql);
    rowCount   = mysql_affected_rows(_mysql);
    fieldCount = mysql_field_count(_mysql);

    if (nullptr == pResult)
    {
        return false;
    }

    if (0 == rowCount)
    {
        mysql_free_result(pResult);
        return false;
    }

    pFields = mysql_fetch_fields(pResult);

    return true;
}

bool IMySqlConnection::Query(PreparedStatementBase *stmt, MySqlPreparedStatement *&pMySqlPreparedStmt,
                             MySqlResult *&pResult,
                             uint64_t &rowCount, uint32_t &fieldCount)
{
    if (nullptr == _mysql)
    {
        return false;
    }

    uint32_t index     = stmt->GetIndex();
    pMySqlPreparedStmt = GetPrepareStatement(index);
    Assert(nullptr != pMySqlPreparedStmt, std::format("尝试获取索引为 {} 的预处理语句错误，请排查原因", index));

    pMySqlPreparedStmt->BindParameters(stmt);
    // pMySqlPreparedStmt = pTmpMySqlPreparedStmt;

    MySqlStmt *pMySqlStmt = pMySqlPreparedStmt->GetMySqlStmt();
    MySqlBind *pMySqlBind = pMySqlPreparedStmt->GetMySqlBind();

    {
        PERFORMANCE_SCOPE(std::format("执行预处理语句 [{}]", pMySqlPreparedStmt->GetSqlString()));

        if (mysql_stmt_bind_param(pMySqlStmt, pMySqlBind))
        {
            uint32_t errcode = mysql_errno(_mysql);
            Log::Error("预处理语句:[{}] 绑定参数出错：{}:{}", pMySqlPreparedStmt->GetSqlString(),
                       errcode, mysql_stmt_error(pMySqlStmt));

            // 成功处理错误，再次尝试执行
            if (HandleMysqlErrcode(errcode))
            {
                return Query(stmt, pMySqlPreparedStmt, pResult, rowCount, fieldCount);
            }

            pMySqlPreparedStmt->ClearParameters();
            return false;
        }

        if (0 != mysql_stmt_execute(pMySqlStmt))
        {
            uint32_t errcode = mysql_errno(_mysql);
            Log::Error("执行预处理语句:[{}] 绑定参数出错：{}:{}", pMySqlPreparedStmt->GetSqlString(),
                       errcode, mysql_stmt_error(pMySqlStmt));

            // 成功处理错误，再次尝试执行
            if (HandleMysqlErrcode(errcode))
            {
                return Query(stmt, pMySqlPreparedStmt, pResult, rowCount, fieldCount);
            }

            pMySqlPreparedStmt->ClearParameters();
            return false;
        }
    }

    pMySqlPreparedStmt->ClearParameters();

    pResult    = mysql_stmt_result_metadata(pMySqlStmt);
    rowCount   = mysql_stmt_num_rows(pMySqlStmt);
    fieldCount = mysql_stmt_field_count(pMySqlStmt);

    return true;
}

void IMySqlConnection::Ping() const
{
    mysql_ping(_mysql);
}
