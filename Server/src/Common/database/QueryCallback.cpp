/*************************************************************************
> File Name       : QueryCallback.cpp
> Brief           : 查询回调
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年08月28日  17时08分22秒
************************************************************************/
#include "pch.h"
#include "QueryCallback.h"
#include "Common/include/Assert.h"

QueryCallback::QueryCallback(QueryResultFuture &&result)
    : _future(std::move(result)), _isPreparedResult(false)
{
}

QueryCallback::QueryCallback(PreparedResultFuture &&result)
    : _preparedFuture(std::move(result)), _isPreparedResult(true)
{
}

QueryCallback::QueryCallback(QueryCallback &&right) noexcept
    : _future(std::move(right._future)), _isPreparedResult(right._isPreparedResult)
{
}

QueryCallback &QueryCallback::operator=(QueryCallback &&right) noexcept
{
    if (this != &right)
    {
        _future           = std::move(right._future);
        _isPreparedResult = right._isPreparedResult;
    }

    return *this;
}

QueryCallback &&QueryCallback::Then(std::function<void(ResultSetPtr)> &&callback)
{
    Assert(!_callbacks.empty() || _isPreparedResult, "错误的将字符串语句查询设置到预处理语句回调上");
    _callbacks.emplace(std::move(callback));
    return std::move(*this);
}

QueryCallback &&QueryCallback::Then(std::function<void(PreparedResultSetPtr)> &&callback)
{
    Assert(!_callbacks.empty() || !_isPreparedResult, "错误的将预处理语句查询设置到字符串语句回调上");
    _callbacks.emplace(std::move(callback));
    return std::move(*this);
}

bool QueryCallback::InvokeIfReady()
{
    QueryCallbackData &callback = _callbacks.front();
    if (_isPreparedResult)
    {
        if (_future.valid() && _future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
        {
            QueryResultFuture                 future(std::move(_future));
            std::function<void(ResultSetPtr)> callbackFunc(std::move(callback._callback));
            callbackFunc(future.get());
        }
    }
    else
    {
        if (_preparedFuture.valid() && _preparedFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
        {
            PreparedResultFuture                      future(std::move(_preparedFuture));
            std::function<void(PreparedResultSetPtr)> callbackFunc(std::move(callback._preparedCallback));
            callbackFunc(future.get());
        }
    }

    return false;
}