#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Entry/ZRenderEntryGeom.h>
#include <Glacier/Geom/ZRoomGeomVolumes.h>


namespace Glacier
{
    class ZRenderEntryRoom : public ZRenderEntryGeom
    {
    public:
        // vtbl
        ~ZRenderEntryRoom() override;
        RENDERENTRY_BASETYPE GetType() const override;

        // methods
        ZRenderEntryRoom(const ZRenderEntryGeomCreateInfo& sInfo);

        static ZRenderEntryRoom* Create(const ZRenderEntryGeomCreateInfo& sInfo);

        uint32_t GetStaticGeomsInRoom(uint32_t* pNumLights, ZBaseGeom** pBaseGeoms, ZBaseGeom** pBaseGeomEnvironments, const ZROOM* pRoom);
        void UpdateMovedGeoms();

        // members
        ZRoomGeomVolumes::ZHeader* m_pStaticVolumesHeader;
        ZRoomGeomVolumes** m_pMovableGeomVolumes;
        uint32_t m_lNumMovableGeomVolumes;
        uint32_t m_lLastFrameUpdate;
    };
    RE_VERIFY_SIZE(ZRenderEntryRoom, 0xACu); // Verified PC alloc
}
