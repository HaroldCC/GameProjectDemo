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

struct PreparedStatementData
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
        data;

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

    constexpr static std::string ToString(bool value)
    {
        return ToString<uint8_t>(value);
    }

    // constexpr static std::string ToString(const std::string &value)
    // {
    //     return value;
    // }

    constexpr static std::string ToString(std::string_view value)
    {
        return value.data();
    }
};

class PreparedStatementBase
{
public:
    PreparedStatementBase(uint32_t preparedStatementIndex, uint8_t dataCapacity);
    ~PreparedStatementBase() = default;

    PreparedStatementBase(const PreparedStatementBase &)            = delete;
    PreparedStatementBase(PreparedStatementBase &&)                 = delete;
    PreparedStatementBase &operator=(const PreparedStatementBase &) = delete;
    PreparedStatementBase &operator=(PreparedStatementBase &&)      = delete;

    template <typename ValueType>
    void SetValue(const uint8_t &index, const ValueType &value);

    [[nodiscard]] uint32_t GetIndex() const
    {
        return _preparedStatementIndex;
    }

    [[nodiscard]] const std::vector<PreparedStatementData> &GetParameters() const
    {
        return _statementData;
    }

private:
    uint32_t                           _preparedStatementIndex; // 预处理语句索引
    std::vector<PreparedStatementData> _statementData;
};

template <typename ConnectionType>
class PreparedStatement : public PreparedStatementBase
{
public:
    using PreparedStatementBase::PreparedStatementBase;
};