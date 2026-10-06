#include <Glacier/Render/ZVolumeList.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZROOM.h>


namespace Glacier
{
    ZBaseGeomVolume** ZVolumeList::BeginGroup(ZGROUP* pGroup)
    {
        ZASSERT(!pGroup || !pGroup->IsDerivedFrom<ZROOM>());
        ZASSERT(m_Groups.m_lNrEntries < MAXNRROOMSSCENE);

        const uint32_t lIndex = m_lCount;

        ZVolumeGroup* pVolumeGroup = &m_Groups.m_Array[m_Groups.m_lNrEntries];
        pVolumeGroup->m_pGroup = pGroup;
        pVolumeGroup->m_lIndex = lIndex;
        pVolumeGroup->m_lCount = 0;
        ++m_Groups.m_lNrEntries;

        return &m_Volumes[lIndex];
    }

    ZBaseGeomVolume** ZVolumeList::BeginRoom(ZROOM* pRoom)
    {
        ZASSERT(static_cast<uint32_t>(pRoom->m_lCacheIndex) < MAXNRROOMSSCENE);

        m_RoomCacheIndexCheck[pRoom->m_lCacheIndex >> 3] |= static_cast<uint8_t>(1u << (pRoom->m_lCacheIndex & 7));

        ZASSERT(m_Groups.m_lNrEntries < MAXNRROOMSSCENE);

        const uint32_t lIndex = m_lCount;

        ZVolumeGroup* pVolumeGroup = &m_Groups.m_Array[m_Groups.m_lNrEntries];
        pVolumeGroup->m_pGroup = pRoom;
        pVolumeGroup->m_lIndex = lIndex;
        pVolumeGroup->m_lCount = 0;
        ++m_Groups.m_lNrEntries;

        m_RoomCacheIndexRemap[pRoom->m_lCacheIndex] = static_cast<uint16_t>(m_Groups.m_lNrEntries - 1);

        return &m_Volumes[lIndex];
    }

    void ZVolumeList::EndGroup(uint32_t lCount)
    {
        ZASSERT(m_Groups.m_lNrEntries > 0);

        m_Groups.m_Array[m_Groups.m_lNrEntries - 1].m_lCount = lCount;
        m_lCount += lCount;

        ZASSERT(m_lCount <= MAX_NUM_VISIBLE_RENDER_ENTRIES);
    }
}
