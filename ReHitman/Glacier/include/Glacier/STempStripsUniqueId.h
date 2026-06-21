#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>

namespace Glacier
{
    union STempStripsUniqueId
    {
        struct
        {
            uint32_t lIdLo;
            uint32_t lIdHi;
        };
        uint64_t lId;
    };
    RE_VERIFY_SIZE(STempStripsUniqueId, 0x8);
}