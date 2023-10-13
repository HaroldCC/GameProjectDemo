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
#include <string>
#include <vector>
#include <variant>
#include <concepts>
#include "Common/include/Assert.h"

enum class SqlArgType : uint32_t
{
    Bool,
    Uint8,
    Uint16,
    Uint32,
    Uint64,
    Int8,
    Int16,
    Int32,
    Int64,
    Float,
    Double,
    String,
    Binary,
    Null,
};

struct PreparedStatementData
{
    std::variant<bool,
                 uint8_t,
                 uint16_t,
                 uint32_t,
                 uint64_t,
                 int8_t,
                 int16_t,
                 int32_t,
                 int64_t,
                 float,
                 double,
                 std::string,
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

    // template <typename... Args>
    // void SetParamValue(const std::vector<SqlArgType> &args, Args &&...value)
    // {
    //     static_assert(args.size() != sizeof...(value), "参数数量与设置参数不匹配");
    //     for (size_t i = 0; i < args.size(); ++i)
    //     {
    //         SetValue(i, args[i], std::forward<Args>(value...));
    //     }
    // }

    template <typename ValueType>
    constexpr void SetValue(const uint8_t &index, SqlArgType type, ValueType &&value)
    {
        Assert(index < _statementData.size());

        using DecayType = std::decay_t<ValueType>;

        if constexpr (std::is_same_v<DecayType, uint8_t>)
        {
            _statementData[index].data.emplace<uint8_t>(std::forward<ValueType>(value));
        }
        else if constexpr (std::is_same_v<DecayType, uint16_t>)
        {
            _statementData[index].data.emplace<uint16_t>(std::forward<ValueType>(value));
        }
        else if constexpr (std::is_same_v<DecayType, uint32_t>)
        {
            _statementData[index].data.emplace<uint32_t>(std::forward<ValueType>(value));
        }
        else if constexpr (std::is_same_v<DecayType, uint64_t>)
        {
            _statementData[index].data.emplace<uint64_t>(std::forward<ValueType>(value));
        }
        else if constexpr (std::is_same_v<DecayType, int8_t>)
        {
            _statementData[index].data.emplace<int8_t>(std::forward<ValueType>(value));
        }
        else if constexpr (std::is_same_v<DecayType, int16_t>)
        {
            _statementData[index].data.emplace<int16_t>(std::forward<ValueType>(value));
        }
        else if constexpr (std::is_same_v<DecayType, int32_t>)
        {
            _statementData[index].data.emplace<int32_t>(std::forward<ValueType>(value));
        }
        else if constexpr (std::is_same_v<DecayType, int64_t>)
        {
            _statementData[index].data.emplace<int64_t>(std::forward<ValueType>(value));
        }
        else if constexpr (std::is_same_v<DecayType, float>)
        {
            _statementData[index].data.emplace<float>(std::forward<ValueType>(value));
        }
        else if constexpr (std::is_same_v<DecayType, double>)
        {
            _statementData[index].data.emplace<double>(std::forward<ValueType>(value));
        }
        else if constexpr (std::is_same_v<DecayType, std::string>)
        {
            _statementData[index].data.emplace<std::string>(std::forward<ValueType>(value));
        }
        // switch (type)
        // {
        // case SqlArgType::Uint8:
        //     _statementData[index].data.emplace<uint8_t>(value);
        // case SqlArgType::Uint16:
        //     _statementData[index].data.emplace<uint16_t>(value);
        // case SqlArgType::Uint32:
        //     _statementData[index].data.emplace<uint32_t>(value);
        // case SqlArgType::Uint64:
        //     _statementData[index].data.emplace<uint64_t>(value);
        // case SqlArgType::Int8:
        //     _statementData[index].data.emplace<int8_t>(value);
        // case SqlArgType::Int16:
        //     _statementData[index].data.emplace<int16_t>(value);
        // case SqlArgType::Int32:
        //     _statementData[index].data.emplace<int32_t>(value);
        // case SqlArgType::Int64:
        //     _statementData[index].data.emplace<int64_t>(value);
        // case SqlArgType::Float:
        //     _statementData[index].data.emplace<float>(std::forward<ValueType>(value));
        // case SqlArgType::Double:
        //     _statementData[index].data.emplace<double>(std::forward<ValueType>(value));
        // case SqlArgType::String:
        //     // _statementData[index].data.emplace<std::string_view>(value);
        //     _statementData[index].data = value;
        // case SqlArgType::Binary:
        //     _statementData[index].data.emplace<std::vector<uint8_t>>(value);
        // case SqlArgType::Null:
        //     _statementData[index].data.emplace<std::nullptr_t>(nullptr);
        //     break;
        // default:
        //     Assert(false, "预处理语句赋值不支持的类型");
        // }
    }

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