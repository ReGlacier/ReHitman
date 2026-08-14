#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    struct SVertexColorD3D
    {
        uint32_t c;
    };
    RE_VERIFY_SIZE(SVertexColorD3D, 0x4);
}
