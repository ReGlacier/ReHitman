#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Prim/SPrimHeader.h>
#include <cstdint>

namespace Glacier
{
    struct SPrims : SPrimHeader
    {
        uint16_t lTextureId;
        uint16_t lDrawEntryId;
        uint32_t lNextPrim;
    };
}