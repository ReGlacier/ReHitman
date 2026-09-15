#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Entry/ZRenderEntryGeom.h>
#include <Glacier/Animation/ZBone.h>


namespace Glacier
{
    class ZRenderEntryBones : public ZRenderEntryGeom
    {
    public:
        // vtbl
        ~ZRenderEntryBones() override;
        void Notify(const SRenderEntryNotifyInfo* pEntry) override;
        void AttachUpdate() override;
        uint32_t GetInstanceRepeat(const ZRenderObjectInstance* pObjInstance) override;
        void SetRenderContext(ZRenderContext* pContext, const ZRenderObjectInstance* pObjInstance) override;
        void EndFrame() override;
        void CalcLODMask(SRenderEntryNotifyInfo* pEntry) override;
        void GetAttachedBaseGeoms(ZStackArray<1024, ZRenderEntry::ZAttachedBaseGeom>* pArray) override;

        // methods
        ZRenderEntryBones(const ZRenderEntryGeomCreateInfo& sInfo);

        static ZRenderEntryBones* Create(const ZRenderEntryGeomCreateInfo& sInfo);

        const ZBone* GetBones() const;
        float* GetBonesLightData() const;
        void SetBonesLightData(float* pLightData);

        // internal methods
        void UpdateActiveNumBones();
        void CreateDefaultBones(uint32_t lFirstBoneNum);

        // members
        char* m_pBoneData;
        float* m_pLightData;
        float m_fMinDistanceToObservers;
        uint32_t m_lNumAllocatedBones;
        uint8_t m_LodLEvelsAvailable;
        RE_ADD_PADDING(3);
    };
    RE_VERIFY_OFFSET(ZRenderEntryBones, m_fMinDistanceToObservers, 0xA4); // PC 0x00477D50 / 0x004B4DA0
    RE_VERIFY_SIZE(ZRenderEntryBones, 0xB0); // Verified by PC ZRenderEntryBonesD3D allocation
}
