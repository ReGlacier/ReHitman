#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>

namespace Glacier
{
    struct STempStripsUniqueId
    {
        union
        {
            struct
            {
                uint32_t lIdLo;
                uint32_t lIdHi;
            } __s0;
            uint64_t lId;
        } ___u0;
    };
    RE_VERIFY_SIZE(STempStripsUniqueId, 0x8);
}