#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/Render/Fwd.h>
#include <Glacier/Render/Draw/ZDrawSurface.h>
#include <Glacier/ZSTL/ZStackArray.h>
#include <cstdint>


namespace Glacier
{
    struct ZDecalCallBack
    {
        // types
        struct ZDecalCallBackEntries
        {
            // members
            ZBaseGeom* m_pBaseGeom;
            float* m_pStoredUV;
            ZPrimAccessMesh* m_pEditableMesh;
        };

        // members
        uint32_t m_lNumEntries;
        ZDecalCallBack::ZDecalCallBackEntries* m_pEntries;
    };

    using DecalViewCallback_t = void(*)(ZDecalCallBack* , void* , uint32_t);

    struct IDraw
    {
        // vtbl
        virtual bool IsValid() = 0;
        virtual bool ShouldFlush() = 0;
        virtual void InitAllocation() = 0;
        virtual int GetLargestFreeBlock() = 0;
        virtual void* Alloc(int lSize, const char* pFile, uint32_t lLine) = 0;
        virtual void Free(void* pMem, int) = 0;
        virtual void* AllocSprites(int lSize) = 0;
        virtual void FreeSprites(void* pMem, int) = 0;
        virtual IView* CreateView(ZRender* pRender, uint32_t lViewId, ZDrawSurface::TARGET lTarget) = 0;
        virtual IView* FindView(uint32_t lViewId) = 0;
        virtual IView* GetViewByIndex(int32_t lViewIndex) = 0;
        virtual bool RemoveView(IView* pView) = 0;
        virtual void RemoveViewsUsingRender(ZRender* pRender) = 0;
        virtual void Init() = 0;
        virtual void End() = 0;
        virtual void Flush() = 0;
        virtual void Update(ZRender* pRender) = 0;
        virtual void BeginFrame() = 0;
        virtual void EndFrame() = 0;
        virtual void Invalidate() = 0;
        virtual uint32_t AddMark(
            const float* vPosition,
            const float* vDirection,
            uint32_t lArrayIndex,
            uint32_t lSourcePrim,
            float fRadiusX, float fRadiusY, 
            const float* fExtraTextureSize, 
            bool bStoreUV, 
            float fRotation) = 0;
        virtual void AddBoneMark(ZBaseGeom* AddBoneMark, const float* vPosition, const float* vDirection, float fRadius, uint32_t lBoneId, uint32_t lSourcePrim) = 0;
        virtual void RemoveMark(uint32_t lHandle) = 0;
        virtual void ModifyMark(uint32_t lHandle, float fOpacity) = 0;
        virtual void SetMarkCallBack(uint32_t lHandle, DecalViewCallback_t pCallBack, void* pCallBackData, uint32_t lCallBackValue) = 0;
        virtual uint32_t GetVisibleBoneObjects(ZBaseGeom** pBoneObjects, int ArraySize) = 0;
        virtual void RemoveBaseGeom(ZBaseGeom* pBaseGeom, bool bRemoveDecals) = 0;
        virtual void UpdateBaseGeom(ZBaseGeom* pBaseGeom) = 0;
        virtual void ChangePrim(ZBaseGeom* pBaseGeom, uint32_t lPrim) = 0;
        virtual bool ValidateCaster(const ZBaseGeom* pBaseGeom) = 0;
        virtual const ZBone* GetBaseGeomBones(const ZBaseGeom* pBaseGeom) = 0;
        virtual bool ValidateReceiver(const ZBaseGeom* pBaseGeom) = 0;
        virtual void EnableOwnerDraw(const ZBaseGeom* pBaseGeom) = 0;
        virtual void DisableOwnerDraw(const ZBaseGeom* pBaseGeom) = 0;
        virtual void GetDrawInfo(ZOldDrawInfo* pOldDrawInfo) = 0;
        virtual void CreateDefaultBones(ZBone* pBone, ZLNKOBJ* pLnkObj) = 0;
        virtual ZPostFilter* FindPostFilterForCamera(ZBaseGeom* pCamera) = 0;
        virtual void UpdateRenderSize(ZRender* pRender, uint32_t lWidth, uint32_t lHeight) = 0;
        virtual void FreeDeviceBuffers() = 0;
        virtual void AllocateDeviceBuffers() = 0;
        virtual ZStackArrayInsert<32, ZRenderViewBase*, uint8_t>* GetViewList() = 0;
        virtual ZRenderViewBase* NewView(ZRender* pRender, uint32_t lViewNumber, uint32_t lViewId) = 0;

        // methods
        static IDraw* Instance();
    };
}