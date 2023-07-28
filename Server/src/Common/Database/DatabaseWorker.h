/*************************************************************************
> File Name       : DatabaseWorker.h
> Brief           : 数据库异步工作线程
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月28日  12时01分46秒
************************************************************************/
#pragma once
#include "Common/threading/ProducerConsumerQueue.hpp"

class SQLOperation;
class MySqlConnection;

class DatabaseWorker
{
public:
    DatabaseWorker(ProducerConsumerQueue<SQLOperation *> *_queue, MySqlConnection *sqlConn);

private:
    void WorkThread();

private:
    MySqlConnection                       *_sqlConn;
    ProducerConsumerQueue<SQLOperation *> *_queue;
    std::thread                            _workerThread;
    std::atomic_bool                       _canceling;
};