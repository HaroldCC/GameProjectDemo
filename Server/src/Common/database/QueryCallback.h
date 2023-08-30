/*************************************************************************
> File Name       : QueryCallback.h
> Brief           : 查询回调
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年08月26日  16时48分39秒
************************************************************************/
#pragma once
#include <variant>
#include <queue>
#include "DatabaseEnv.h"

class QueryCallback
{
public:
    explicit QueryCallback(QueryResultFuture &&result);
    explicit QueryCallback(PreparedResultFuture &&result);

    QueryCallback(QueryCallback &&right) noexcept;
    QueryCallback &operator=(QueryCallback &&right) noexcept;
    ~QueryCallback() = default;

    QueryCallback &&Then(std::function<void(ResultSetPtr)> &&callback);
    QueryCallback &&Then(std::function<void(PreparedResultSetPtr)> &&callback);

    QueryCallback(const QueryCallback &)            = delete;
    QueryCallback &operator=(const QueryCallback &) = delete;

    bool InvokeIfReady();

private:
    /**
     * @brief 有两种查询，一种是直接执行sql语句的查询，
     *        一种是预处理语句查询，相应的，返回就有两种
     */
    union
    {
        QueryResultFuture    _future;
        PreparedResultFuture _preparedFuture;
    };
    bool _isPreparedResult; // 是否预处理查询结果

    // for Then
    struct QueryCallbackData
    {
        /**
         * @brief 有两种查询，一种是直接执行sql语句的查询，
         *        一种是预处理语句查询，相应的，返回就有两种
         */
        union
        {
            std::function<void(ResultSetPtr)>         _callback;
            std::function<void(PreparedResultSetPtr)> _preparedCallback;
        };
        bool _isPreparedResult; // 是否预处理查询结果

        explicit QueryCallbackData(std::function<void(ResultSetPtr)> &&callback)
            : _callback(std::move(callback)), _isPreparedResult(false)
        {
        }

        explicit QueryCallbackData(std::function<void(PreparedResultSetPtr)> &&callback)
            : _preparedCallback(std::move(callback)), _isPreparedResult(true)
        {
        }
    };
    std::queue<QueryCallbackData> _callbacks;
};