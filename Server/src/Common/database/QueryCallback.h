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
#include "Common/include/AsyncCallbackProcessor.h"

class QueryCallback
{
public:
    explicit QueryCallback(QueryResultFuture &&result);
    explicit QueryCallback(PreparedResultFuture &&result);
    ~QueryCallback() = default;

    QueryCallback(QueryCallback &&right) noexcept;
    QueryCallback &operator=(QueryCallback &&right) noexcept;

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
    std::variant<QueryResultFuture, PreparedResultFuture> _future;
    bool                                                  _isPreparedResult; // 是否预处理查询结果

    // for Then
    struct QueryCallbackData
    {
        /**
         * @brief 有两种查询，一种是直接执行sql语句的查询，
         *        一种是预处理语句查询，相应的，返回就有两种
         */
        using ResultSetCallback         = std::function<void(ResultSetPtr)>;
        using PreparedResultSetCallback = std::function<void(PreparedResultSetPtr)>;
        std::variant<ResultSetCallback, PreparedResultSetCallback> _callback;
        bool                                                       _isPreparedResult; // 是否预处理查询结果

        explicit QueryCallbackData(std::function<void(ResultSetPtr)> &&callback)
            : _callback(std::move(callback)), _isPreparedResult(false)
        {
        }

        explicit QueryCallbackData(std::function<void(PreparedResultSetPtr)> &&callback)
            : _callback(std::move(callback)), _isPreparedResult(true)
        {
        }
    };
    std::queue<QueryCallbackData> _callbacks;
};

using QueryCallbackProcessor = AsyncCallbackProcessor<QueryCallback>;