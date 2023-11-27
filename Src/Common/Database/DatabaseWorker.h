/*************************************************************************
> File Name       : DatabaseWorker.h
> Brief           : 数据库异步工作线程
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月28日  12时01分46秒
************************************************************************/
#pragma once
#include "Common/Threading/ProducerConsumerQueue.hpp"

class ISqlTask;
class IMySqlConnection;

class DatabaseWorker
{
public:
    DatabaseWorker(ProducerConsumerQueue<ISqlTask *> *queue, IMySqlConnection *sqlConn);
    ~DatabaseWorker();
    DatabaseWorker(const DatabaseWorker &)            = delete;
    DatabaseWorker(DatabaseWorker &&)                 = delete;
    DatabaseWorker &operator=(const DatabaseWorker &) = delete;
    DatabaseWorker &operator=(DatabaseWorker &&)      = delete;

private:
    void WorkThread();

private:
    IMySqlConnection                  *_sqlConn;
    ProducerConsumerQueue<ISqlTask *> *_queue;
    std::thread                        _workerThread;
    std::atomic_bool                   _canceling;
};