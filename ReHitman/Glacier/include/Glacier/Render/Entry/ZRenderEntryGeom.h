#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Entry/ZRenderEntryGeomCreateInfo.h>
#include <Glacier/Render/Entry/ZRenderEntry.h>
#include <cstdint>


namespace Glacier
{
    class ZRenderEntryGeom : public ZRenderEntry
    {
    public:
        // vtbl
        ~ZRenderEntryGeom() override;
        void Notify(const SRenderEntryNotifyInfo* pEntry) override;
        void Update() override;
        void AttachUpdate() override;
        void SetRenderContext(ZRenderContext* pContext, const ZRenderObjectInstance* pObjInstance) override;
        RENDERENTRY_BASETYPE GetType() const override;
        ZBaseGeom* GetBaseGeom() const override;
        uint32_t GetPrim() const override;

        // methods
        ZRenderEntryGeom(const ZRenderEntryGeomCreateInfo& sInfo);

        // members
        uint32_t m_lUnknown80;                          // 0x80
        float m_fVertexFrameNumber;                     // 0x84 - Verified PC ZPrimControlBase::SetPrimFrameNr
        uint32_t m_lRenderFlags;                        // 0x88 - I'm not sure about this! Need R&D later
        ZBaseGeom* m_pBaseGeom;                         // 0x8C
        uint32_t m_lPrimId;                             // 0x90
        uint32_t m_lUnknown94;                          // 0x94
        uint32_t m_lUnknown98;                          // 0x98
    };
    RE_VERIFY_SIZE(ZRenderEntryGeom, 0x9C); // Verifued by ZRenderEntryGeomD3D ctor
}
