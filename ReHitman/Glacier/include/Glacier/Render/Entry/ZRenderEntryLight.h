#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Prim/ZPrimHandle.h>
#include <Glacier/Render/Entry/ZRenderEntryGeom.h>


namespace Glacier
{
    class ZRenderEntryLight : public ZRenderEntryGeom
    {
    public:
        // vtbl
        ~ZRenderEntryLight() override;
        void Update() override;
        RENDERENTRY_BASETYPE GetType() const override;
        virtual const ZPrimHandle& GetPrimLight() const;

        // methods
        ZRenderEntryLight(const ZRenderEntryGeomCreateInfo& sInfo);

        static ZRenderEntryLight* Create(const ZRenderEntryGeomCreateInfo& sInfo);

        // members
        ZPrimHandle m_hPrimLight;
    };
}
