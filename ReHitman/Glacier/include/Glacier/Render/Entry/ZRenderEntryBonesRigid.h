#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Entry/ZRenderEntryBones.h>

namespace Glacier
{
    class ZRenderEntryBonesRigid : public ZRenderEntryBones
    {
    public:
        ~ZRenderEntryBonesRigid() override;
        void __thiscall Update() override;
        void SetRenderContext(ZRenderContext* pContext, const ZRenderObjectInstance* pObjInstance) override;
        uint32_t GetHiddenBoneIndices() const override;
        uint32_t GetBoneIndexMask(const SPrimObject* pPrimObject) override;

        ZRenderEntryBonesRigid(const ZRenderEntryGeomCreateInfo& sInfo);
        static ZRenderEntryBonesRigid* Create(const ZRenderEntryGeomCreateInfo& sInfo);

        uint32_t m_lHiddenBoneIndices;
    };

    RE_VERIFY_OFFSET(ZRenderEntryBonesRigid, m_lHiddenBoneIndices, 0xB0);
    RE_VERIFY_SIZE(ZRenderEntryBonesRigid, 0xB4);
}
