/*************************************************************************
> File Name       : PreparedStatement.h
> Brief           : MySql预处理语句
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年08月03日  18时24分49秒
************************************************************************/
#pragma once
#include <format>
#include <string_view>
#include <vector>
#include <variant>
#include <concepts>

struct PrepareStatementData
{
    std::variant<bool,
                 uint8_t,
                 uint16_t,
                 uint32_t,
                 uint64_t,
                 int8_t,
                 int16_t,
                 int64_t,
                 float,
                 double,
                 std::string_view,
                 std::vector<uint8_t>,
                 std::nullptr_t>
        _data;

    template <typename T>
        requires std::is_standard_layout_v<T> && std::is_trivial_v<T>
    constexpr static std::string ToString(T &&value)
    {
        return std::format("{}", std::forward<T>(value));
    }

    constexpr static std::string ToString(const std::vector<uint8_t> & /*unused*/)
    {
        return "BINARY";
    }

    constexpr static std::string ToString(std::nullptr_t)
    {
        return "NULL";
    }
};

class PrepareStatementBase
{
public:
    PrepareStatementBase(uint32_t paramIndex, uint8_t dataCapacity);
    ~PrepareStatementBase() = default;

    PrepareStatementBase(const PrepareStatementBase &)            = delete;
    PrepareStatementBase(PrepareStatementBase &&)                 = delete;
    PrepareStatementBase &operator=(const PrepareStatementBase &) = delete;
    PrepareStatementBase &operator=(PrepareStatementBase &&)      = delete;

private:
    uint32_t                          _paramIndex;
    std::vector<PrepareStatementData> _statementData;
};