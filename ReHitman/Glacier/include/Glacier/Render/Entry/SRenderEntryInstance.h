#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
#include <cstdint>


namespace Glacier
{
    struct SRenderEntryInstance
    {
        ZRenderObjectInstance* pRenderObjectInstance;
        uint32_t lSortValue;
        uint32_t lLayerMask;
        uint32_t lBoneIndexMask;
        uint8_t lLODMask;
        uint8_t lDrawDestination;
        uint8_t lTransparencyMask;
        uint8_t lFlags;
    };
}