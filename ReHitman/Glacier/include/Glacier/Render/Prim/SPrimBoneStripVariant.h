#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Prim/SPrimDrawSetup.h>
#include <cstdint>


namespace Glacier
{
    struct SPrimBoneStripVariant : SPrimDrawSetup
    {
        uint32_t lPrim;
    };
    RE_VERIFY_SIZE(SPrimBoneStripVariant, 0x20);
}