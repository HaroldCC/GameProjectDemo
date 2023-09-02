/*************************************************************************
> File Name       : DatabaseWorker.cpp
> Brief           : 数据库异步工作线程
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年07月29日  16时37分14秒
************************************************************************/
#include "pch.h"
#include <memory>
#include "DatabaseWorker.h"
#include "SqlTask.h"

DatabaseWorker::DatabaseWorker(ProducerConsumerQueue<ISqlTask *> *queue, IMySqlConnection *sqlConn)
    : _queue(queue), _sqlConn(sqlConn), _canceling(false)
{
    _workerThread = std::thread(&DatabaseWorker::WorkThread, this);
}

DatabaseWorker::~DatabaseWorker()
{
    _canceling = true;
    _queue->Clear();
    if (_workerThread.joinable())
    {
        _workerThread.join();
    }
}

void DatabaseWorker::WorkThread()
{
    if (nullptr == _queue)
    {
        return;
    }

    while (true)
    {
        std::unique_ptr<ISqlTask> operation(_queue->WaitAndPop());
        if (_canceling || nullptr == operation)
        {
            return;
        }

        operation->SetConnection(_sqlConn);
        operation->Execute();
    }
}