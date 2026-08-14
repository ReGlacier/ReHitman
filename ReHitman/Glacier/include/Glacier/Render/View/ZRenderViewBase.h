#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZStackArray.h>
#include <Glacier/Render/View/IView.h>
#include <Glacier/Render/Fwd.h>
#include <cstdint>


namespace Glacier
{
    class ZRenderViewBase : public IView
    {
    public:
        // constants
        static constexpr int MAX_VIEWS_NR = 0x20;
        // overrides TBD
        ZRender* Render() override;
        void Init() override;
        void End() override;
        const uint32_t* Viewport() override;
        void EnablePostfilter() override;
        void RemoveCameras() override;
        void SetViewport(const uint32_t* vp) override;
        void SetOverrideScissor(const uint32_t*) override;
        void AddCamera(ZCAMERA*) override;
        void RemoveCamera(ZCAMERA*) override;
        ZCAMERA* GetCamera(int) override;
        void CreateSurface(ZDrawSurface::TARGET) override;
        void GetDrawInfo(ZOldDrawInfo*) override;
        bool IsCameraInView(ZBaseGeom*) override;
        virtual void FreeDeviceBuffers();
        virtual void AllocateDeviceBuffers();
        virtual void CreatePostFilter();
        virtual ZPostFilter* GetPostFilter();
        virtual ZPostFilter* FindPostFilterForCamera(ZBaseGeom* pCameraGeom);

        // methods
        ZRenderViewBase(ZRender *pRender, IDraw *pDraw, unsigned int lViewNumber, unsigned int lViewId);

        // members
        uint32_t m_alNumTriangles[4];
        uint32_t m_alNumPrimitives[4];
        uint32_t m_alNumDrawCalls[4];
        ZRender* m_pRender;
        uint32_t m_lViewPort[4];
        uint32_t m_lScissorOverride[4];
        uint32_t m_lViewNumber;
        uint32_t m_lViewId;
        ZStackArrayInsert<16, ZCAMERA*, int> m_Cameras; // +0x60 - Verified
        bool m_bPostfilterEnabled;
        RE_ADD_PADDING(3);
        ZPostFilter* m_pPostFilter; // +0xE8 - Verified
        bool m_bOwnerDraw;
        RE_ADD_PADDING(3);
        ZDrawSurface* m_pDrawSurface;
    };
    RE_VERIFY_SIZE(ZRenderViewBase, 0xF4); // I'm sure but no proofs lol
    RE_VERIFY_OFFSET(ZRenderViewBase, m_lViewPort, 0x38); // Verified ZRenderViewBase::SetViewport
    RE_VERIFY_OFFSET(ZRenderViewBase, m_Cameras, 0x60);
    RE_VERIFY_OFFSET(ZRenderViewBase, m_pPostFilter, 0xE8);
}