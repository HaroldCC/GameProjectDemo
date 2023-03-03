module;

#include "spdlog/spdlog.h"

export module common;

class SourceLocation
{
public:
    constexpr SourceLocation(const char *fileName = __builtin_FILE(), const char *funcName = __builtin_FUNCTION(),
                             unsigned int lineNum = __builtin_LINE()) noexcept
        : _fileName(fileName), _funcName(funcName), _lineNum(lineNum) {}

    constexpr const char        *FileName() const noexcept { return _fileName; }
    constexpr const char        *FuncName() const noexcept { return _funcName; }
    constexpr const unsigned int LineNum() const noexcept { return _lineNum; }

private:
    const char        *_fileName;
    const char        *_funcName;
    const unsigned int _lineNum;
};

inline constexpr auto GetLogSourceLocation(const SourceLocation &location)
{
    return spdlog::source_loc{location.FileName(), static_cast<int>(location.LineNum()), location.FuncName()};
}

class CLog
{
public:
    CLog &GetLogger()
    {
        static CLog logger;
        return logger;
    }

    CLog(const CLog &)            = delete;
    CLog &operator=(const CLog &) = delete;
    CLog(CLog &&)                 = delete;

private:
    CLog()  = default;
    ~CLog() = default;
};

export namespace logger
{
    // info
    template <typename... Args>
    struct info
    {
        constexpr info(fmt::format_string<Args...> fmt, Args &&...args, SourceLocation location = {})
        {
            spdlog::log(GetLogSourceLocation(location), spdlog::level::info, fmt, std::forward<Args>(args)...);
        }
    };

    template <typename... Args>
    info(fmt::format_string<Args...> fmt, Args &&...args) -> info<Args...>;

    // error
    template <typename... Args>
    struct error
    {
        constexpr error(fmt::format_string<Args...> fmt, Args &&...args, SourceLocation location = {})
        {
            spdlog::log(GetLogSourceLocation(location), spdlog::level::err, fmt, std::forward<Args>(args)...);
        }
    };

    template <typename... Args>
    error(fmt::format_string<Args...> fmt, Args &&...args) -> error<Args...>;

    // debug
    template <typename... Args>
    struct debug
    {
        constexpr debug(fmt::format_string<Args...> fmt, Args &&...args, SourceLocation location = {})
        {
            spdlog::log(GetLogSourceLocation(location), spdlog::level::debug, fmt, std::forward<Args>(args)...);
        }
    };

    template <typename... Args>
    debug(fmt::format_string<Args...> fmt, Args &&...args) -> debug<Args...>;

    // trace
    template <typename... Args>
    struct trace
    {
        constexpr trace(fmt::format_string<Args...> fmt, Args &&...args, SourceLocation location = {})
        {
            spdlog::log(GetLogSourceLocation(location), spdlog::level::trace, fmt, std::forward<Args>(args)...);
        }
    };

    template <typename... Args>
    trace(fmt::format_string<Args...> fmt, Args &&...args) -> trace<Args...>;
}
