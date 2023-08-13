/*************************************************************************
> File Name       : DatabaseWorkerPool.h
> Brief           : 数据库工作池
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年08月03日  15时27分43秒
************************************************************************/
#pragma once
#include <cstdint>
#include <string_view>

template <typename ConnectionType>
class PreparedStatement;

template <typename ConnectionType>
class DatabaseWorkerPool
{
public:
    DatabaseWorkerPool();
    ~DatabaseWorkerPool();

    DatabaseWorkerPool(const DatabaseWorkerPool &)            = delete;
    DatabaseWorkerPool(DatabaseWorkerPool &&)                 = delete;
    DatabaseWorkerPool &operator=(const DatabaseWorkerPool &) = delete;
    DatabaseWorkerPool &operator=(DatabaseWorkerPool &&)      = delete;

    uint32_t Open();
    void     Close();

    bool PrepareStatements();

    void AsyncExecute(std::string_view sql);
    void AsyncExecute(PreparedStatement<ConnectionType> *stmt);

    void SyncExecute(std::string_view sql);
    void SyncExecute(PreparedStatement<ConnectionType> *stmt);

private:
};