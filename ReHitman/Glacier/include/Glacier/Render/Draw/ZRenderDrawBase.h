#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
#include <Glacier/Render/Draw/IDraw.h>
#include <Glacier/ZSTL/ZStackArray.h>
#include <Glacier/ZUniMemory.h>
#include <cstdint>


namespace Glacier
{
    class ZRenderDrawBase : public IDraw
    {
    public:
        // vtbl
        bool IsValid() override;
        bool ShouldFlush() override;
        void InitAllocation() override;
        int GetLargestFreeBlock() override;
        void* Alloc(int lSize, const char* pFile, uint32_t lLine) override;
        void Free(void* pMem, int) override;
        void* AllocSprites(int lSize) override;
        void FreeSprites(void* pMem, int) override;
        IView* CreateView(ZRender* pRender, uint32_t lViewId, ZDrawSurface::TARGET lTarget) override;
        IView* FindView(uint32_t lViewId) override;
        IView* GetViewByIndex(int32_t lViewIndex) override;
        bool RemoveView(IView* pView) override;
        void RemoveViewsUsingRender(ZRender* pRender) override;
        void Init() override;
        void End() override;
        void Flush() override;
        void Update(ZRender* pRender) override;
        void BeginFrame() override;
        void EndFrame() override;
        void Invalidate() override;
        void AddBoneMark(ZBaseGeom* AddBoneMark, const float* vPosition, const float* vDirection, float fRadius, uint32_t lBoneId, uint32_t lSourcePrim) override;
        void UpdateBaseGeom(ZBaseGeom* pBaseGeom) override;
        void ChangePrim(ZBaseGeom* pBaseGeom, uint32_t lPrim) override;
        bool ValidateCaster(const ZBaseGeom* pBaseGeom) override;
        const ZBone* GetBaseGeomBones(const ZBaseGeom* pBaseGeom) override;
        bool ValidateReceiver(const ZBaseGeom* pBaseGeom) override;
        void EnableOwnerDraw(const ZBaseGeom* pBaseGeom) override;
        void DisableOwnerDraw(const ZBaseGeom* pBaseGeom) override;
        void GetDrawInfo(ZOldDrawInfo* pOldDrawInfo) override;
        void CreateDefaultBones(ZBone* pBone, ZLNKOBJ* pLnkObj) override;
        ZPostFilter* FindPostFilterForCamera(ZBaseGeom* pCamera) override;
        void UpdateRenderSize(ZRender* pRender, uint32_t lWidth, uint32_t lHeight) override;
        void FreeDeviceBuffers() override;
        void AllocateDeviceBuffers() override;
        ZStackArrayInsert<MAX_VIEWS_NR, ZRenderViewBase*, uint8_t>* GetViewList() override;
        
        // methods
        ZRenderDrawBase();

        // members
        bool m_bValid;
        bool m_bInitialized;
        RE_ADD_PADDING(2);
        IView* m_ViewNumberToPointer[MAX_VIEWS_NR];
        ZStackArrayInsert<MAX_VIEWS_NR, ZRenderViewBase*, uint8_t> m_Views;
    };
}