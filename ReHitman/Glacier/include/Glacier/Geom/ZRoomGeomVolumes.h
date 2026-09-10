#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZStackArray.h>
#include <Glacier/ZSTL/ZMath.h>


namespace Glacier
{
    // fwds
    class ZROOM;
    class ZBaseGeom;

    class ZRoomGeomVolumes
    {
    public:
        // types
        struct ZHeader
        {
            // methods
            // members
            uint32_t m_lNumGeoms;
            uint32_t m_lNumLights;
            uint32_t m_lNumEnvironments;
            uint32_t m_Pad;
        };

        struct ZAttachedGeom
        {
            // methods
            // members
            ZBaseGeom* m_pBaseGeom;
            ZMatrix m_RootPosition;
        };

        struct ZVolumesBlock
        {
            // constants
            static constexpr auto BLOCK_SIZE = 0x20000;

            // methods
            void* Alloc(uint32_t lByteSize);

            // members
            ZRoomGeomVolumes::ZVolumesBlock* m_pNext;
            uint32_t m_Pad[3];
            char m_Memory[BLOCK_SIZE];
            char* m_pCurrent;
        };

        // methods
        ZRoomGeomVolumes();
        ~ZRoomGeomVolumes();
        ZRoomGeomVolumes::ZHeader* CreateGroupVolumes(ZGROUP* pGroup);
        ZRoomGeomVolumes::ZHeader* GetVolumesHeader(ZROOM* pRoom, bool bExitsEnabled);
        uint32_t GetGroupGeomsEnvironmentsRecursive(ZGROUP* pGroup, ZBaseGeom** pBaseGeoms, ZBaseGeom** pBaseGeomEnvironments, ZBaseGeom* pRoomEnvironment, ZBaseGeom* pRoomCharacterEnvironment);
        void* Alloc(uint32_t lByteSize);
        uint32_t GetGeomsInRoom(uint32_t* pNumLights, uint32_t* pNumEnvironments, ZBaseGeom** pBaseGeoms, ZBaseGeom** pBaseGeomEnvironments, ZROOM* pRoom, bool bExitsEnabled);

        // members
        ZStackArray<1024, ZROOM*> m_Rooms{};
        ZRoomGeomVolumes::ZVolumesBlock* m_pFirstBlock{nullptr};
        ZRoomGeomVolumes::ZVolumesBlock m_DefaultBlock{};
    };
    RE_VERIFY_SIZE(ZRoomGeomVolumes, 0x2101C);
}
