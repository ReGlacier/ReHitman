#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
//#include <Glacier/Render/Decal/ZDecalMarkController.h>
#include <Glacier/Render/Draw/ZRenderDrawBase.h>


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
        virtual void CreateRenderEntryInstance(ZPrimHandle const&,ZRenderEntry *,ZBaseGeom *,bool);
        virtual void DestroyRenderEntryInstance(SRenderEntryInstance*);
        virtual void CleanupUnused();
        virtual void CalcBoneLightSources(ZBaseGeom* pBaseGeom, float*) = 0;
        virtual void CalcBoneLightSources(ZRenderEntryBones* pEntryBones, float*);

        // methods

        ZRenderEntry* AddRenderEntryArray(uint32_t lPrim, const SDrawArray* pDrawArray);
        ZRenderEntrySprite* AddRenderEntrySprite(uint32_t lPrim);

        // members
    };
}