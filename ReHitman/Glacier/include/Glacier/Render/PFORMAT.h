#pragma once

#include <cstdint>


namespace Glacier
{
    enum ZPFORMAT : uint32_t
    {
        ZPF1555 = 0x0,
        ZPF4444 = 0x1,
        ZPF0888 = 0x2,
        ZPF8888 = 0x3,
        ZPFABGR = 0x4,
        ZPFP256 = 0x5,
        ZPFP16 = 0x6,
        ZPF_FORCE_LONG = 0x7FFFFFFF,
    };
}