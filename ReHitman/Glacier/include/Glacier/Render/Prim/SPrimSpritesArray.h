#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Prim/SPrims.h>
#include <cstdint>


namespace Glacier
{
    struct SPrimSpritesArray : SPrims
    {
        uint32_t lSpriteType;
        uint32_t lDrawMode;
    };
    RE_VERIFY_SIZE(SPrimSpritesArray, 0x14);
}