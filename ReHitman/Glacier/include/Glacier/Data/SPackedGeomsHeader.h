#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    struct SPackedGeomsHeader
    {
        union
        {
            uint32_t StartQuad[8];
            struct
            {
                uint32_t lEntriesOffset; // Points to struct { u32 lTotalGeoms; SPackedGeomsTree aGeoms[lTotalGeoms]; }
                uint32_t lVersion1; // expected 0
                uint32_t lVersion2; // expected 0
                uint32_t lVersion3; // expected 4
                uint32_t lGeomStatsOffset;
                uint32_t lGeomTypesOffset;
                uint32_t lLightOffset;
                uint32_t lEventDataOffset;
            };
        };
        uint32_t m_lGeomResourcesHeadersOffset;
        uint32_t m_lGeomResourcesOffset;
        uint32_t m_lRoomEnvironmentListOffset;
        uint32_t m_iCollisionDataOffset;
        uint32_t m_iMaterialDescOffset;
        uint32_t m_lOffsetPathfinder4Data;
        uint32_t m_lPhysicsDataOffset;
        uint32_t m_iHighestGeomNr;
        uint32_t m_iWeaponPrimsOffset;
        uint32_t m_iExcludedAnimNamesOffset;
    };
    RE_VERIFY_SIZE(SPackedGeomsHeader, 0x48);
}
