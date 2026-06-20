#pragma once

#include <cstdint>

namespace Glacier
{
    enum ITEMSTATE : uint32_t {
        eIS_NORMAL = 1,
        IS_HIDE = 2,
        IS_SHOW = 4,
        IS_ACTIVATE = 8,
        IS_ACTIVATE2 = 16,
        IS_EXTRA1 = 32,
        IS_EXTRA2 = 64,
        IS_LASTITEM = 128,
        IS_FORCE32 = 2147483647u
    };
}