#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Fwd.h>
#include <Glacier/Render/D3D9.h>
#include <Glacier/Render/Draw/ZDrawSurface.h>


namespace Glacier
{
    class ZDrawSurfaceD3D : public ZDrawSurface
    {
    public:
        // vtbl
        ~ZDrawSurfaceD3D() override;
        void Begin() override;
        void End() override;
        void SetViewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h) override;

        // methods
        ZDrawSurfaceD3D(ZDrawSurface::TARGET lTarget, ZRenderWintelD3D* pRender);

        // members
        IDirect3DTexture9* m_pColorTexture;
        IDirect3DSurface9* m_pColorSurface;
        IDirect3DSurface9* m_pDepthSurface;
        IDirect3DSurface9* m_pColorSurfaceOld;
        IDirect3DSurface9* m_pDepthSurfaceOld;
        D3DVIEWPORT9 m_ViewportOld;
        ZRenderWintelD3D* m_pRender;
    };
    RE_VERIFY_SIZE(ZDrawSurfaceD3D, 0x38); // Verified PC
    RE_VERIFY_OFFSET(ZDrawSurfaceD3D, m_pColorTexture, 0x8); // Verified by ZDrawSurfaceD3D::m_pColorTexture
    RE_VERIFY_OFFSET(ZDrawSurfaceD3D, m_pColorSurface, 0xC); // Verified by ZDrawSurfaceD3D::Begin
    RE_VERIFY_OFFSET(ZDrawSurfaceD3D, m_pDepthSurface, 0x10); // Verified by ZDrawSurfaceD3D::Begin
    RE_VERIFY_OFFSET(ZDrawSurfaceD3D, m_pColorSurfaceOld, 0x14); // Verified by ZDrawSurfaceD3D::Begin
    RE_VERIFY_OFFSET(ZDrawSurfaceD3D, m_pDepthSurfaceOld, 0x18); // Verified by ZDrawSurfaceD3D::Begin
    RE_VERIFY_OFFSET(ZDrawSurfaceD3D, m_ViewportOld, 0x1C); // Verified by ZDrawSurfaceD3D::Begin
}