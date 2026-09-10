#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZStackArray.h>
#include <cstdint>


namespace Glacier
{
    // fwds
    class ZBaseGeomVolume;
    class ZGROUP;

    class ZVolumeList
    {
    public:
        // constants
        static constexpr auto MAX_NUM_VISIBLE_RENDER_ENTRIES = 0x800;

        // types
        struct ZVolumeGroup
        {
            // members
            ZGROUP* m_pGroup;
            uint32_t m_lIndex;
            uint32_t m_lCount;
        };

        // methods
        ZBaseGeomVolume** BeginRoom(ZROOM* pRoom);
        ZBaseGeomVolume** BeginGroup(ZGROUP* pGroup);
        void EndGroup(uint32_t lCount);

        // members
        ZStackArray<1024, ZVolumeList::ZVolumeGroup> m_Groups;
        ZBaseGeomVolume* m_Volumes[MAX_NUM_VISIBLE_RENDER_ENTRIES];
        uint16_t m_RoomCacheIndexRemap[1024];
        uint8_t m_RoomCacheIndexCheck[128];
        uint32_t m_lCount;
    };
}
