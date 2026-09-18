#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Prim/SPrimStripsWintel.h>
#include <cstdint>


namespace Glacier
{
    struct SPrimSplinesWintel : SPrimStripsWintel
    {
        uint32_t lSplineVertices; // 0x7C
        uint32_t lSplineIndices;  // 0x80
    };
    RE_VERIFY_SIZE(SPrimSplinesWintel, 0x84);
}
