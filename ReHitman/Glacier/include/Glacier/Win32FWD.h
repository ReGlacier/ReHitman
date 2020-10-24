#pragma once

#include <cstdint>

namespace Glacier
{
    // Win32 API defs
    struct SYSTEMTIME
    {
        uint16_t wYear;
        uint16_t wMonth;
        uint16_t wDayOfWeek;
        uint16_t wDay;
        uint16_t wHour;
        uint16_t wMinute;
        uint16_t wSecond;
        uint16_t wMilliseconds;
    };

    struct FILETIME {
        uint32_t dwLowDateTime;
        uint32_t dwHighDateTime;
    };
}