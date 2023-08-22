/*************************************************************************
> File Name       : MySqlConnection.cpp
> Brief           : 数据库连接
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月27日  15时37分16秒
************************************************************************/
#include "pch.h"
#include "MySqlConnection.h"
#include "Common/include/Log.hpp"
#include "Common/include/Performance.hpp"
#include "mysqld_error.h"
#include "DatabaseWorker.h"
#include "PreparedStatement.h"
#include "MySqlPreparedStatement.h"

using namespace std::literals::chrono_literals;

IMySqlConnection::IMySqlConnection(const MySqlConnectionInfo &connInfo)
    : _connectionInfo(connInfo),
      _mysql(nullptr),
      _reconnecting(false),
      _prepareError(false),
      _connectType(ConnectionType::SYNC),
      _queue(nullptr)
{
}

IMySqlConnection::IMySqlConnection(ProducerConsumerQueue<SQLOperation *> *queue, const MySqlConnectionInfo &connInfo)
    : _connectionInfo(connInfo),
      _mysql(nullptr),
      _reconnecting(false),
      _prepareError(false),
      _connectType(ConnectionType::ASYNC),
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
        Timer timer;
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
bool IMySqlConnection::Execute(PreparedStatementBase *stmt)
{
    if (nullptr == _mysql)
    {
        return false;
    }

    uint32_t index = stmt->GetIndex();
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