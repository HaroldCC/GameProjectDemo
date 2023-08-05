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
