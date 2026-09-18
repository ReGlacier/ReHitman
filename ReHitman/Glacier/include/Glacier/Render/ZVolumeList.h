#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Geom/ZRoomGeomVolumes.h>
#include <Glacier/ZSTL/ZStackArray.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    // fwds
    class ZGROUP;
    class ZROOM;

    class ZVolumeList
    {
    public:
        // constants
        static constexpr auto MAX_NUM_VISIBLE_RENDER_ENTRIES = 0x800;
        static constexpr auto MAXNRROOMSSCENE = 0x400;

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
        ZStackArray<MAXNRROOMSSCENE, ZVolumeList::ZVolumeGroup> m_Groups;          // 0x0000
        ZBaseGeomVolume* m_Volumes[MAX_NUM_VISIBLE_RENDER_ENTRIES];                // 0x3004
        uint16_t m_RoomCacheIndexRemap[MAXNRROOMSSCENE];                           // 0x5004
        uint8_t m_RoomCacheIndexCheck[MAXNRROOMSSCENE / 8];                        // 0x5804
        uint32_t m_lCount;                                                         // 0x5884
    };
    RE_VERIFY_SIZE(ZVolumeList, 0x5888);
    RE_VERIFY_OFFSET(ZVolumeList, m_Groups, 0x0);
    RE_VERIFY_OFFSET(ZVolumeList, m_Volumes, 0x3004);
    RE_VERIFY_OFFSET(ZVolumeList, m_RoomCacheIndexRemap, 0x5004);
    RE_VERIFY_OFFSET(ZVolumeList, m_RoomCacheIndexCheck, 0x5804);
    RE_VERIFY_OFFSET(ZVolumeList, m_lCount, 0x5884);
}
