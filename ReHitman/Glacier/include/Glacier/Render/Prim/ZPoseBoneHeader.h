#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    struct ZPoseBoneHeader
    {
        uint32_t m_PoseData;
        uint32_t m_PoseIndexToOffsetLookup;
        int32_t m_PoseBones;
        uint32_t m_PoseIdToPosLookup;
        int32_t m_PoseCount;
    };
    RE_VERIFY_SIZE(ZPoseBoneHeader, 0x14);
}
