#pragma once

#include <Glacier/Render/Entry/ZRenderEntryDeformer.h>
#include <Glacier/Render/Prim/ZPrimHandle.h>

namespace Glacier
{
    class ZCloth;

    class ZRenderEntryDeformerD3D : public ZRenderEntryDeformer
    {
    public:
        ~ZRenderEntryDeformerD3D() override;
        void Notify(const SRenderEntryNotifyInfo* pEntry) override;
        void Update() override;
        void GetVisible(ZCmdList*, ZRenderEntryGeom*, ZViewSpace*, ZRenderView*, ZRenderEntryLists*) override;

        ZRenderEntryDeformerD3D(const ZRenderEntryGeomCreateInfo& sInfo);

        static ZRenderEntryDeformerD3D* Create(ZRenderEntryGeomCreateInfo& sInfo);

        uint32_t m_lUnknown9C;       // +0x9C
        ZPrimHandle m_hPrimAccess; // +0xA0, PC stores the locked sub-primitive handle
        ZCloth* m_pCloth;           // +0xA4
    };

    RE_VERIFY_OFFSET(ZRenderEntryDeformerD3D, m_hPrimAccess, 0xA0);
    RE_VERIFY_OFFSET(ZRenderEntryDeformerD3D, m_pCloth, 0xA4);
    RE_VERIFY_SIZE(ZRenderEntryDeformerD3D, 0xA8);
}
