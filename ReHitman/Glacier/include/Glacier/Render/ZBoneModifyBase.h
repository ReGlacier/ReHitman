#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    // fwds
    class ZLNKOBJ;
    struct ZBone;

    class ZBoneModifyBase
    {
    public:
        // methods
        uint8_t DecalLookup() const { return m_lDecalLookup; }
        const ZBone* GetBones(const ZLNKOBJ* pLnkObj) const;

        // members
        uint8_t m_pad00[0x12];  // +0x00 (not reversed yet)
        uint8_t m_lDecalLookup; // +0x12 (offset approved by PS2 & XBOX builds)
        // tail is not reversed yet
    };
}
