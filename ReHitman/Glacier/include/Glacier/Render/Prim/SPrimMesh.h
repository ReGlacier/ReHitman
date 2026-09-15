#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Prim/SPrimObject.h>
#include <cstdint>


namespace Glacier
{
    struct SPrimMesh : SPrimObject
    {
        uint32_t lSubMeshTable;
        uint32_t lNumFrames;
        uint16_t lFrameStart;
        uint16_t lFrameStep;
        uint32_t lTrisPerStripColor;
    };
    RE_VERIFY_SIZE(SPrimMesh, 0x38);
}