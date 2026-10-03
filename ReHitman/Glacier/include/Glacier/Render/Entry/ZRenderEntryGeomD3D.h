#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Entry/ZRenderEntryGeom.h>
#include <Glacier/Render/Entry/ZRenderEntryGeomCreateInfo.h>
#include <cstdint>


namespace Glacier
{
    class ZRenderEntryGeomD3D : public ZRenderEntryGeom
    {
    public:
        // vtbl
        ~ZRenderEntryGeomD3D() override;
        void Update() override;

        // methods
        using ZRenderEntryGeom::ZRenderEntryGeom;

        static ZRenderEntryGeomD3D* Create(ZRenderEntryGeomCreateInfo& sCreationInfo);
    };
}
