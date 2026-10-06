#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    struct SPrimVariant
    {
        uint32_t m_lPrim;
        uint32_t m_lVariantId;
        uint32_t m_lTextureId;
    };
    RE_VERIFY_SIZE(SPrimVariant, 0xC);
}