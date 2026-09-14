#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Entry/ZRenderEntryGeom.h>


namespace Glacier
{
    class ZRenderEntryCamera : public ZRenderEntryGeom
    {
    public:
        // vtbl
        ~ZRenderEntryCamera() override;
        void Update() override;
        RENDERENTRY_BASETYPE GetType() const override;

        // methods
        ZRenderEntryCamera(const ZRenderEntryGeomCreateInfo& sInfo);

        // members
        uint32_t m_lDrawConForbid;
    };
    RE_VERIFY_OFFSET(ZRenderEntryCamera, m_lDrawConForbid, 0x9C);
    RE_VERIFY_SIZE(ZRenderEntryCamera, 0xA0);
}
