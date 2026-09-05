#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
#include <cstdint>


namespace Glacier
{
    struct SRenderEntryInstance
    {
        ZRenderObjectInstance* pRenderObjectInstance; // +0x0
        uint32_t lSortValue;                          // +0x4
        uint32_t lLayerMask;                          // +0x8
        uint8_t lBoneIndexMask;                       // +0xC (byte on PC; XBox has 4 extra tail bytes)
        uint8_t lLODMask;                             // +0xD
        uint8_t lDrawDestination;                     // +0xE
        uint8_t lTransparencyMask;                    // +0xF
    };
    RE_VERIFY_SIZE(SRenderEntryInstance, 0x10); // Approved by PC pool stride (ZRenderDraw::Ctor free-list, DestroyRenderEntryInstance)
}
