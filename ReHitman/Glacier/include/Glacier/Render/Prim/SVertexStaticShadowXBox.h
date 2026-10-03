#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    // XBOX_KL1 static-shadow stream: three signed fixed-point coordinates and
    // one grayscale vertex color. Keep this byte-packed; it is source data,
    // not a render-buffer layout.
#pragma pack(push, 1)
    struct SVertexStaticShadowXBox
    {
        int16_t x;
        int16_t y;
        int16_t z;
        uint8_t c;
    };
#pragma pack(pop)
    RE_VERIFY_SIZE(SVertexStaticShadowXBox, 0x7);
}
