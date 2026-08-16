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
        ZRenderEntryGeom(ZRenderEntryGeomCreateInfo* pInfo);

        // members
        uint32_t m_lUnknown80;
        uint32_t m_lUnknown84;
        uint32_t m_lUnknown88;
        ZBaseGeom* m_pBaseGeom;
        uint32_t m_lPrimId;
        uint32_t m_lUnknown94;
        uint32_t m_lUnknown98;
    };

    // RE_VERIFY_OFFSET(ZRenderEntryGeom, m_pBaseGeom, 0x8C); // Verified PC ctor
    // RE_VERIFY_OFFSET(ZRenderEntryGeom, m_lPrimId, 0x90); // Verified PC ctor
}