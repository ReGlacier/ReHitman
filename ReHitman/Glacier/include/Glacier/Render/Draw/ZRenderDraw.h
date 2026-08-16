#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
#include <Glacier/Render/Draw/ZRenderDrawBase.h>
#include <Glacier/Render/Decal/ZDecalMarkController.h>
#include <Glacier/Render/Entry/SRenderEntryInstance.h>
#include <Glacier/ZSTL/ZAllocIndex.h>
#include <Glacier/ZSTL/ZFixedArray.h>
#include <Glacier/ZSTL/STLport.h>


namespace Glacier
{
    class ZRenderDraw : public ZRenderDrawBase
    {
    public:
        // vtbl
        void Flush() override;
        uint32_t AddMark(
            const float* vPosition,
            const float* vDirection,
            uint32_t lArrayIndex,
            uint32_t lSourcePrim,
            float fRadiusX, float fRadiusY, 
            const float* fExtraTextureSize, 
            bool bStoreUV, 
            float fRotation) override;
        void AddBoneMark(ZBaseGeom* AddBoneMark, const float* vPosition, const float* vDirection, float fRadius, uint32_t lBoneId, uint32_t lSourcePrim) override;
        void RemoveMark(uint32_t lHandle) override;
        void ModifyMark(uint32_t lHandle, float fOpacity) override;
        void SetMarkCallBack(uint32_t lHandle, DecalViewCallback_t pCallBack, void* pCallBackData, uint32_t lCallBackValue) override;
        uint32_t GetVisibleBoneObjects(ZBaseGeom** pBoneObjects, int ArraySize) override;
        void RemoveBaseGeom(ZBaseGeom* pBaseGeom, bool bRemoveDecals) override;
        void UpdateBaseGeom(ZBaseGeom* pBaseGeom) override;
        void ChangePrim(ZBaseGeom* pBaseGeom, uint32_t lPrim) override;
        bool ValidateCaster(const ZBaseGeom* pBaseGeom) override;
        const ZBone* GetBaseGeomBones(const ZBaseGeom* pBaseGeom) override;
        bool ValidateReceiver(const ZBaseGeom* pBaseGeom) override;

        virtual void WaitRenderDone();
        virtual void SetTextureFrameNumber(ZBaseGeom const*,float);
        virtual SRenderEntryInstance* CreateRenderEntryInstance(ZPrimHandle const&,ZRenderEntry *,ZBaseGeom *,bool);
        virtual void DestroyRenderEntryInstance(SRenderEntryInstance*);
        virtual void CleanupUnused();
        virtual void CalcBoneLightSources(ZBaseGeom* pBaseGeom, float*) = 0;
        virtual void CalcBoneLightSources(ZRenderEntryBones* pEntryBones, float*);

        // methods
        ZRenderDraw();

        ZRenderEntry* AddRenderEntryArray(uint32_t lPrim, const SDrawArray* pDrawArray);
        ZRenderEntrySprite* AddRenderEntrySprite(uint32_t lPrim);

        // members
        ZDecalMarkController m_DecalMarks;                        // +0x12C. Name approved by XBox PDB
        ZRenderEntry* m_apRenderEntryLookup[0x8000];              // +0x7F70. m_lDrawId -> render entry (zeroed in Flush)
        ZAllocIndex m_RenderEntryIndex;                           // +0x27F70. Draw id allocator (15 bits, ids are index + 1)
        uint32_t m_lRenderEntriesCount;                           // +0x27F84
        ZRenderEntry* m_apRenderEntries[0x8000];                  // +0x27F88. All live render entries
        stlp::map<uint32_t, ZRenderObject*> m_RenderObjects;      // +0x47F88. Prim handle -> shared render object
        uint32_t m_lToBeDeletedCount;                             // +0x47F94
        ZRenderObjectInstance* m_apToBeDeleted[512];              // +0x47F98. Object instances pending deletion
        RE_ADD_PADDING(4);                                        // +0x48798
        ZFixedArray<SRenderEntryInstance, 4096> m_RenderEntryInstances; // +0x4879C. Name approved by XBox PDB
        RE_ADD_PADDING(0xC4);                                     // +0x587A0 - 0x58864. Unused tail (not touched by ctor/Flush/CleanupUnused)
    };
    RE_VERIFY_OFFSET(ZRenderDraw, m_DecalMarks, 0x12C);            // Approved by PC & XBox ctor
    RE_VERIFY_OFFSET(ZRenderDraw, m_apRenderEntryLookup, 0x7F70);  // Approved by PC Flush (0x20000 byte memset)
    RE_VERIFY_OFFSET(ZRenderDraw, m_RenderEntryIndex, 0x27F70);    // Approved by PC ctor
    RE_VERIFY_OFFSET(ZRenderDraw, m_lRenderEntriesCount, 0x27F84); // Approved by PC Flush
    RE_VERIFY_OFFSET(ZRenderDraw, m_apRenderEntries, 0x27F88);     // Approved by PC Flush
    RE_VERIFY_OFFSET(ZRenderDraw, m_RenderObjects, 0x47F88);       // Approved by PC ctor (24 byte rb-tree nodes)
    RE_VERIFY_OFFSET(ZRenderDraw, m_lToBeDeletedCount, 0x47F94);   // Approved by PC DestroyRenderEntryInstance
    RE_VERIFY_OFFSET(ZRenderDraw, m_apToBeDeleted, 0x47F98);       // Approved by PC DestroyRenderEntryInstance
    RE_VERIFY_OFFSET(ZRenderDraw, m_RenderEntryInstances, 0x4879C);// Approved by PC ctor (16 byte stride, 4096 entries)
    RE_VERIFY_SIZE(ZRenderDraw, 0x58864);                          // Approved by ZSharedResourcesD3D::ZSharedResourcesD3D allocation
}
