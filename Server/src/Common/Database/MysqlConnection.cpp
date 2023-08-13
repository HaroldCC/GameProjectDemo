/*************************************************************************
> File Name       : MysqlConnection.cpp
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

using namespace std::literals::chrono_literals;

MySqlConnection::MySqlConnection(const MySqlConnectionInfo &connInfo)
    : _connectionInfo(connInfo),
      _mysql(nullptr),
      _reconnecting(false),
      _prepareError(false),
      _connectType(ConnectionType::SYNC),
      _queue(nullptr)
{
}

MySqlConnection::MySqlConnection(ProducerConsumerQueue<SQLOperation *> *queue, const MySqlConnectionInfo &connInfo)
    : _connectionInfo(connInfo),
      _mysql(nullptr),
      _reconnecting(false),
      _prepareError(false),
      _connectType(ConnectionType::ASYNC),
      _queue(queue)
{
    _worker = std::make_unique<DatabaseWorker>(queue, this);
}

/**
 * @brief 开启连接
 *
 * @return uint32_t 错误码（ref mysql_errno）
 */
uint32_t MySqlConnection::Open()
{
    _mysql = mysql_init(nullptr);
    if (nullptr == _mysql)
    {
        Log::error("初始化数据库：{}失败", _connectionInfo._database);
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
        Log::error("连接数据库失败:{} [host:{}, user:{}, password:{}, database:{}, port:{}]", mysql_error(_mysql),
                   _connectionInfo._host, _connectionInfo._user,
                   _connectionInfo._password, _connectionInfo._database, port);
        mysql_close(_mysql);
        return errcode;
    }

    if (!_reconnecting)
    {
        Log::info("Mysql client library:{}", mysql_get_client_info());
        Log::info("Mysql server version:{}", mysql_get_server_info(_mysql));
    }

    mysql_autocommit(_mysql, true);
    mysql_set_character_set(_mysql, "utf8mb4");
    return 0;
}

void MySqlConnection::Close()
{
    if (nullptr != _mysql)
    {
        mysql_close(_mysql);
        _mysql = nullptr;
    }
}

bool MySqlConnection::PrepareStatements()
{
    DoPrepareStatements();
    return !_prepareError;
}

bool MySqlConnection::Execute(std::string_view sql)
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
            Log::info("执行sql脚本：SQL:{}", sql);
            Log::error("执行sql脚本出错：[{}]{}", errcode, mysql_error(_mysql));

            // 处理mysql错误，如果返回true，表示成功处理，继续尝试执行
            if (HandleMysqlErrcode(errcode))
            {
                return Execute(sql);
            }

            return false;
        }

        Log::debug("执行脚本成功，耗时：{} ms SQL:{}", timer.ElapsedMillisec(), sql);
    }

    return true;
}

bool MySqlConnection::TryLock()
{
    return _mutex.try_lock();
}

void MySqlConnection::UnLock()
{
    _mutex.unlock();
}

bool MySqlConnection::HandleMysqlErrcode(uint32_t errcode, uint8_t tryReconnectTimes /* = 5 */)
{
    switch (errcode)
    {
    case CR_SERVER_GONE_ERROR:
    case CR_SERVER_LOST:
    case CR_SERVER_LOST_EXTENDED:
    {
        if (nullptr != _mysql)
        {
            Log::error("Mysql server 连接丢失");
            mysql_close(_mysql);
            _mysql = nullptr;
        }
    }
    case CR_CONN_HOST_ERROR:
    {
        Log::info("Reconnecting Mysql server......");
        _reconnecting          = true;
        const uint32_t errcode = Open();
        if (0 != errcode)
        {
            if (!this->PrepareStatements())
            {
                Log::critical("处理预处理sql语句失败，数据库连接即将断开!!!");
                std::this_thread::sleep_for(10s);
                std::abort();
            }

            Log::info("重连数据库成功：{} @{}:{} {}", _connectionInfo._database,
                      _connectionInfo._host, _connectionInfo._port,
                      ((uint8_t)_connectType & (uint8_t)ConnectionType::ASYNC) ? "异步方式" : "同步方式");
            _reconnecting = false;
            return true;
        }

        if ((--tryReconnectTimes) == 0)
        {
            Log::critical("多次尝试重连数据库失败，服务中断！！！");
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
        Log::error("数据库未找到对应的表");
        std::this_thread::sleep_for(std::chrono::seconds(10));
        std::abort();
        return false;
    case ER_PARSE_ERROR:
        Log::error("数据库脚本出错，请检查语法");
        std::this_thread::sleep_for(std::chrono::seconds(10));
        std::abort();
        return false;
    default:
    {
        Log::error("未处理的Mysql 错误，错误码：{}", errcode);
        return false;
    }
    }

    return false;
}