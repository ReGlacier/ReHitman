#include <Glacier/Geom/ZRoomGeomVolumes.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    void* ZRoomGeomVolumes::ZVolumesBlock::Alloc(uint32_t lByteSize)
    {
        // TODO: Finish me
        return nullptr;
    }

    ZRoomGeomVolumes::ZRoomGeomVolumes() = default;

    ZRoomGeomVolumes::~ZRoomGeomVolumes()
    {
        // TODO: Finish me
    }

    ZRoomGeomVolumes::ZHeader* ZRoomGeomVolumes::CreateGroupVolumes(ZGROUP* pGroup)
    {
        // TODO: Finish me
        return nullptr;
    }

    ZRoomGeomVolumes::ZHeader* ZRoomGeomVolumes::GetVolumesHeader(ZROOM* pRoom, bool bExitsEnabled)
    {
        // TODO: Finish me
        return nullptr;
    }

    uint32_t ZRoomGeomVolumes::GetGroupGeomsEnvironmentsRecursive(ZGROUP* pGroup, ZBaseGeom** pBaseGeoms, ZBaseGeom** pBaseGeomEnvironments, ZBaseGeom* pRoomEnvironment, ZBaseGeom* pRoomCharacterEnvironment)
    {
        // TODO: Finish me
        return 0;
    }

    void* ZRoomGeomVolumes::Alloc(uint32_t lByteSize)
    {
        // TODO: Finish me
        return nullptr;
    }

    uint32_t ZRoomGeomVolumes::GetGeomsInRoom(uint32_t* pNumLights, uint32_t* pNumEnvironments, ZBaseGeom** pBaseGeoms, ZBaseGeom** pBaseGeomEnvironments, ZROOM* pRoom, bool bExitsEnabled)
    {
        // TODO: Finish me
        return 0;
    }
}
