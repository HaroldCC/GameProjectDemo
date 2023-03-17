module;

#include <format>
#include <string_view>
#include "include/platform.h"

export module common:util;

export namespace util
{
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
} // namespace util