#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    struct SPrimVariantArray
    {
        uint32_t m_lNumPrimsInVariant;
        uint32_t m_lPrimVariants;
    };
    RE_VERIFY_SIZE(SPrimVariantArray, 0x8);
}