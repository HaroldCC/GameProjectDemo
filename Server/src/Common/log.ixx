/*************************************************************************
> File Name       : log.ixx
> Brief           : 日志模块
> Author          : Harold
> Mail            : 2106562095@qq.com
> Github          : www.github.com/Haroldcc
> Created Time    : 2023年03月03日  15时20分51秒
************************************************************************/

module;

#include <vector>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_sinks.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/rotating_file_sink.h"

export module common.log;

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

export namespace logger
{
    class CLogger final
    {
    public:
        static CLogger &GetLogger()
        {
            static CLogger logger;
            return logger;
        }

        CLogger(const CLogger &)            = delete;
        CLogger &operator=(const CLogger &) = delete;
        CLogger(CLogger &&)                 = delete;

        void SetLevel(spdlog::level::level_enum level)
        {
            _logger->set_level(level);
        }

    private:
        CLogger() = default;

        void InitLogger(std::string_view fileName, unsigned int level, size_t maxFileSize, size_t maxFiles)
        {
            auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(fileName, 1024 * 1024 * 5, 10);
            fileSink->set_level(spdlog::level::trace);

            auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            consoleSink->set_level(spdlog::level::trace);

            std::vector<spdlog::sink_ptr> sinks{fileSink, consoleSink};
            _logger = std::make_shared<spdlog::logger>("MultiLogger", std::begin(sinks), std::end(sinks));

            spdlog::set_default_logger(_logger);
        }

        ~CLogger() = default;

    private:
        std::shared_ptr<spdlog::logger> _logger;
    };

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
