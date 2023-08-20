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
#include <array>
#include <memory>
#include "QueryResult.h"

template <typename ConnectionType>
class PreparedStatement;

template <typename ConnectionType>
class DatabaseWorkerPool
{
private:
    enum EConnectionTypeIndex
    {
        EConnectionTypeIndex_Async,
        EConnectionTypeIndex_Sync,
        EConnectionTypeIndex_Max,
    };

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
    void AsyncExecute(PreparedStatement<ConnectionType> *pStmt);

    void SyncExecute(std::string_view sql);
    void SyncExecute(PreparedStatement<ConnectionType> *pStmt);

    ResultSet         AsyncQuery(std::string_view sql);
    PreparedResultSet AsyncQuery(PreparedStatement<ConnectionType> *pStmt);

    ResultSet         SyncQuery(std::string_view sql, ConnectionType *pConnection = nullptr);
    PreparedResultSet SyncQuery(PreparedStatement<ConnectionType> *pStmt);

private:
    ConnectionType *GetFreeConnection();

private:
    using ConnectionPtr = std::unique_ptr<ConnectionType>;
    std::array<std::vector<ConnectionPtr>, EConnectionTypeIndex_Max> _connections;
};