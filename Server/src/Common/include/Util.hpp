#pragma once
#include "Platform.h"
#include <string_view>
#include <type_traits>
#include <chrono>
#include <limits>
#include "Assert.h"

namespace Util
{
    template <typename T>
    class Singleton
    {
    public:
        // 使用了consteval来保证在编译时就初始化单例对象，避免了运行时的等待和竞争条件。
        consteval static T &Instance()
        {
            static_assert(std::is_default_constructible_v<T>, "单例需要实现默认构造函数");
            static T instance;
            return instance;
        }

        Singleton(Singleton &&)                 = delete;
        Singleton &operator=(Singleton &&)      = delete;
        Singleton(const Singleton &)            = delete;
        Singleton &operator=(const Singleton &) = delete;

    protected:
        Singleton()  = default;
        ~Singleton() = default;
    };

    inline bool ProgressCanRun(const std::string_view strProgressName)
    {
#ifdef OS_PLATFORM_WINDOWS
        HANDLE hMutex = nullptr;
        hMutex        = ::CreateMutex(nullptr, FALSE, strProgressName.data());
        if (nullptr != hMutex)
        {
            if (::GetLastError() == ERROR_ALREADY_EXISTS)
            {
                ::CloseHandle(hMutex);
                return true;
            }
        }

        return false;
#else
        std::int32_t fd          = -1;
        const char   szBuf[32]   = {0};
        std::string  strLockFile = std::format("/var/run/{}.pid", strProgressName);
        fd                       = ::open(strLockFile.c_str(), O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        if (fd < 0)
        {
            return true;
        }
        struct flock fl;
        fl.l_type   = F_WRLCK;
        fl.l_start  = 0;
        fl.l_whence = SEEK_SET;
        fl.l_len    = 0;

        if (::fcntl(fd, F_SETLK, &fl) < 0)
        {
            ::close(fd);
            return true;
        }

        ::ftruncate(fd, 0);

        snprintf(szBuf, 32, "%ld", (long)::getpid());

        ::write(fd, szBuf, strlen(szBuf) + 1);

        return false;
#endif
    }

    template <size_t N>
    struct ChangeResult
    {
        char Data[N];
    };

    template <size_t N, size_t K>
    constexpr auto CleanupOutputString(const char (&expr)[N], const char (&remove)[K])
    {
        ChangeResult<N> result = {};

        size_t srcIndex = 0;
        size_t dstIndex = 0;
        while (srcIndex < N)
        {
            size_t matchIndex = 0;
            while (matchIndex < K - 1 &&
                   srcIndex + matchIndex < N - 1 &&
                   expr[srcIndex + matchIndex] == remove[matchIndex])
            {
                ++matchIndex;
            }

            if (matchIndex == K - 1)
            {
                srcIndex += matchIndex;
            }

            result.Data[dstIndex++] = expr[srcIndex] == '"' ? '\'' : expr[srcIndex];
            ++srcIndex;
        }

        return result;
    }

/**
 * @brief enum class To underlying type
 *
 */
#if defined(__cpp_lib_to_underlying)
    using ToUnderlying = std::to_underlying;
#else
    template <typename Enum>
    inline constexpr std::underlying_type_t<Enum> ToUnderlying(Enum eum) noexcept
    {
        return static_cast<std::underlying_type_t<Enum>>(eum);
    }
#endif

    const auto                                                g_AppStartTime = std::chrono::steady_clock::now();
    inline std::chrono::time_point<std::chrono::steady_clock> GetAppStartTime()
    {
        return g_AppStartTime;
    }

    inline uint64_t GetMillSecTimeNow()
    {
        return uint64_t(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - GetAppStartTime()).count());
    }

    inline uint64_t GetMillSecTimeDiff(uint64_t oldTime, uint64_t newTime)
    {
        if (oldTime > newTime)
        {
            return ((std::numeric_limits<uint64_t>::max)() - oldTime) + newTime;
        }

        return newTime - oldTime;
    }

    inline uint64_t GetMillSecTimeDiffToNow(uint64_t oldTime)
    {
        return GetMillSecTimeDiff(oldTime, GetMillSecTimeNow());
    }

    inline bool StringEqual(std::string_view str1, std::string_view str2)
    {
        return std::equal(str1.begin(), str1.end(), str2.begin(), str2.end(), [](char char1, char char2)
                          { return std::tolower(char1) == std::tolower(char2); });
    }

    namespace StringConvertImpl
    {
        template <typename T>
        concept IntegralType = std::is_integral_v<T> && !std::is_same_v<T, bool>;

        template <IntegralType T>
        struct For
        {
            constexpr static std::optional<T> FromString(std::string_view str, int base = 10)
            {
                if (0 == base)
                {
                    if (StringEqual(str.substr(0, 2), "0x"))
                    {
                        base = 16;
                        str.remove_prefix(2);
                    }
                    else if (StringEqual(str.substr(0, 2), "0b"))
                    {
                        base = 2;
                        str.remove_prefix(2);
                    }
                    else
                    {
                        base = 10;
                    }

                    if (str.empty())
                    {
                        return std::nullopt;
                    }
                }

                const char *const begin = str.data();
                const char *const end   = (begin + str.length());
                T                 val;
                auto [ptr, errc] = std::from_chars(begin, end, val, base);
                if (ptr == end && errc == std::errc{})
                {
                    return val;
                }

                return std::nullopt;
            }

            constexpr static std::string ToString(T val)
            {
                using bufferSize = std::integral_constant<size_t, sizeof(T) < 8 ? 11 : 20>;

                std::string buf(bufferSize::value, '\0');
                char *const beg  = buf.data();
                char *const end  = (beg + buf.length());
                auto [ptr, errc] = std::to_chars(beg, end, val);
                Assert(errc == std::errc{});
                buf.resize(ptr - beg);
                return buf;
            }
        };

        struct ForBool
        {
            constexpr static std::optional<bool> FromString(std::string_view str, int strict = 0)
            {
                if (strict)
                {
                    if (str == "1")
                    {
                        return true;
                    }
                    if (str == "0")
                    {
                        return false;
                    }

                    return std::nullopt;
                }

                if ((str == "1") || StringEqual(str, "y") || StringEqual(str, "on") || StringEqual(str, "yes"))
                {
                    return true;
                }

                if ((str == "0") || StringEqual(str, "n") || StringEqual(str, "off") || StringEqual(str, "no"))
                {
                    return false;
                }

                return std::nullopt;
            }
            constexpr static std::string ToString(bool val)
            {
                return val ? "1" : "0";
            }
        };

        template <typename T>
            requires std::is_floating_point_v<T>
        struct ForFloat
        {
            constexpr static std::optional<T> FromString(std::string_view str, std::chars_format charFmt = std::chars_format{})
            {
                if (str.empty())
                {
                    return std::nullopt;
                }

                if (charFmt == std::chars_format{})
                {
                    if (StringEqual(str.substr(0, 2), "0x"))
                    {
                        charFmt = std::chars_format::hex;
                        str.remove_prefix(2);
                    }
                    else
                    {
                        charFmt = std::chars_format::general;
                    }

                    if (str.empty())
                    {
                        return std::nullopt;
                    }
                }

                const char *beg = str.data();
                const char *end = (beg + str.length());
                T           val{};
                auto [ptr, errc] = std::from_chars(beg, end, val, charFmt);
                if (ptr == end && errc == std::errc{})
                {
                    return val;
                }

                return std::nullopt;
            }

            constexpr static std::optional<T> FromString(std::string_view str, int base)
            {
                if (16 == base)
                {
                    return FromString(str, std::chars_format::hex);
                }

                if (10 == base)
                {
                    return FromString(str, std::chars_format::general);
                }

                return FromString(str, std::chars_format{});
            }

            constexpr static std::string ToString(T val)
            {
                return std::to_string(val);
            }
        };

    } // namespace StringConvertImpl

    template <typename Result, typename... Params>
    constexpr std::optional<Result> StringTo(std::string_view str, Params &&...params)
    {
        if constexpr (std::is_same_v<Result, bool>)
        {
            return StringConvertImpl::ForBool::FromString(str, std::forward<Params>(params)...);
        }
        else if constexpr (std::is_floating_point_v<Result>)
        {
            return StringConvertImpl::ForFloat<Result>::FromString(str, std::forward<Params>(params)...);
        }
        else
        {
            return StringConvertImpl::For<Result>::FromString(str, std::forward<Params>(params)...);
        }
    }

    template <typename Type, typename... Params>
    std::string ToString(Type &&val, Params &&...params)
    {
        if constexpr (std::is_same_v<Type, bool>)
        {
            return StringConvertImpl::ForBool::ToString(std::forward<Type>(val), std::forward<Params>(params)...);
        }
        else if constexpr (std::is_floating_point_v<std::decay_t<Type>>)
        {
            return StringConvertImpl::ForFloat<std::decay_t<Type>>::ToString(std::forward<Type>(val), std::forward<Params>(params)...);
        }
        else
        {
            return StringConvertImpl::For<std::decay_t<Type>>::ToString(std::forward<Type>(val), std::forward<Params>(params)...);
        }
    }

} // namespace Util