#pragma once
#include "platform.h"
#include <ranges>
#include <vector>
#include <string_view>
#include <type_traits>
#include <utility>

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

    bool ProgressCanRun(const std::string_view strProgressName)
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
    constexpr std::underlying_type_t<Enum> ToUnderlying(Enum eum) noexcept
    {
        return static_cast<std::underlying_type_t<Enum>>(eum);
    }
#endif

} // namespace Util