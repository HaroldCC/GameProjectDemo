/*************************************************************************
> File Name       : DatabaseWorkerPool.cpp
> Brief           : 数据库工作池
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年08月03日  15时30分14秒
************************************************************************/
#include "pch.h"
#include "DatabaseWorkerPool.h"

template <typename ConnectionType>
DatabaseWorkerPool<ConnectionType>::DatabaseWorkerPool()
{
}

template <typename ConnectionType>
DatabaseWorkerPool<ConnectionType>::~DatabaseWorkerPool()
{
}

template <typename ConnectionType>
uint32_t DatabaseWorkerPool<ConnectionType>::Open()
{
}

template <typename ConnectionType>
void DatabaseWorkerPool<ConnectionType>::Close()
{
}

template <typename ConnectionType>
ResultSet DatabaseWorkerPool<ConnectionType>::AsyncQuery(std::string_view sql)
{
}

template <typename ConnectionType>
ResultSet DatabaseWorkerPool<ConnectionType>::SyncQuery(std::string_view sql, ConnectionType *pConnection /* = nullptr */)
{
    if (nullptr == pConnection)
    {
        pConnection = GetFreeConnection();
    }

    ResultSet *pResult = pConnection->Query(sql);
    pConnection->Unlock();
}

template <typename ConnectionType>
ConnectionType *DatabaseWorkerPool<ConnectionType>::GetFreeConnection()
{
    const auto      connectionCount = _connections[EConnectionTypeIndex_Sync].size();
    ConnectionType *pConnection     = nullptr;
    uint8_t         index           = 0;
    while (nullptr == pConnection)
    {
        pConnection = _connections[EConnectionTypeIndex_Sync][index++ % connectionCount].get();
        if (pConnection->TryLock())
        {
            break;
        }
    }

    return pConnection;
}
