#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Entry/ZRenderEntryCamera.h>
#include <Glacier/ZSTL/ZStackArray.h>
#include <Glacier/Render/D3D9.h>


namespace Glacier
{
    class ZRenderEntryCameraD3D : public ZRenderEntryCamera
    {
    public:
        // vtbl
        ~ZRenderEntryCameraD3D() override;
        void GetVisible(ZCmdList* pCmdList, ZRenderEntryGeom* pGeomEntry, ZViewSpace* pViewSpace, ZRenderView* pView, ZRenderEntryLists* pEntryList) override;

        // methods
        ZRenderEntryCameraD3D(const ZRenderEntryGeomCreateInfo& sInfo);
        static ZRenderEntryCameraD3D* Create(const ZRenderEntryGeomCreateInfo& sInfo);

        void BuildDrawChainDropShadows(ZRenderEntryGeom* pParentRenderEntryGeom, ZRenderView* pRenderView, ZCmdList* pCmdList, ZRenderEntryLists* pGeomList, uint32_t* plMaxProjections);
        void BuildDrawChainStaticShadowTextureProjections(
            ZCmdList* pCmdList,
            uint32_t lMaxProjections,
            ZStackArray<1024u, ZRenderEntryGeom*>& LnkList,
            ZRenderView* pRenderView,
            const ZVector3& vObserver,
            float fLODScale,
            uint32_t lType,
            uint32_t lLayer);
        float CalculateShadowIntensity(const ZBaseGeom* pLight, const ZBaseGeom* pCaster);

        // members
        D3DXMATRIX m_matProjection;
        uint32_t m_lNumShadows;
    };
    RE_VERIFY_SIZE(ZRenderEntryCameraD3D, 0xE4u); // Verified PC alloc
}
