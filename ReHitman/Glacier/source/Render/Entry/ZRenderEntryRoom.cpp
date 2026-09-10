#include <Glacier/Render/Entry/ZRenderEntryRoom.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZRenderEntryRoom::ZRenderEntryRoom(const ZRenderEntryGeomCreateInfo& sInfo)
        : ZRenderEntryGeom(sInfo)
    {
        // TODO: Finish me
    }

    ZRenderEntryRoom::~ZRenderEntryRoom()
    {
        // TODO: Finish me
    }

    ZRenderEntryRoom* ZRenderEntryRoom::Create(const ZRenderEntryGeomCreateInfo& sInfo)
    {
        // TODO: Finish me
        return ZUniMemory::New<ZRenderEntryRoom>(sInfo);
    }

    ZRenderEntryRoom::RENDERENTRY_BASETYPE ZRenderEntryRoom::GetType() const
    {
        return ZRenderEntryRoom::RENDERENTRY_BASETYPE::RT_ROOM; // 6
    }

    uint32_t ZRenderEntryRoom::GetStaticGeomsInRoom(uint32_t* pNumLights, ZBaseGeom** pBaseGeoms, ZBaseGeom** pBaseGeomEnvironments, const ZROOM* pRoom)
    {
        // TODO: Finish me
        return 0;
    }

    void ZRenderEntryRoom::UpdateMovedGeoms()
    {
        // TODO: Finish me
    }
}
