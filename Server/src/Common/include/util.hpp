﻿#pragma once
#include "platform.h"
#include <ranges>
#include <vector>
#include <string_view>

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

    template <template <typename> typename Container = std::vector, typename Arg = std::string_view>
    auto Split(std::string_view str, std::string_view delimiter)
    {
        Container<Arg> container;
        auto           temp = str |
                    std::ranges::views::split(delimiter) |
                    std::ranges::views::transform(
                        [](auto &&word)
                        { return Arg(std::addressof(*word.begin()), std::ranges::distance(word)); });
        auto iter = std::inserter(container, container.end());
        std::ranges::for_each(temp, [&](auto &&word)
                              { iter = {word.begin(), word.end()}; });
        return container;
    }
} // namespace Util