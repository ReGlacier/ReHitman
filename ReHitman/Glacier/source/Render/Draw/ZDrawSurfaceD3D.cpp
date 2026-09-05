#include <Glacier/Render/Draw/ZDrawSurfaceD3D.h>
#include <Glacier/Render/ZDirect3DDevice.h>
#include <Glacier/Render/Globals.h>


namespace Glacier
{
    ZDrawSurfaceD3D::ZDrawSurfaceD3D(ZDrawSurface::TARGET lTarget, ZRenderWintelD3D* pRender)
        : ZDrawSurface(lTarget)
        , m_pRender(pRender)
        , m_pColorTexture(nullptr)
        , m_pColorSurface(nullptr)
        , m_pDepthSurface(nullptr)
        , m_pColorSurfaceOld(nullptr)
        , m_pDepthSurfaceOld(nullptr)
    {
    }

    ZDrawSurfaceD3D::~ZDrawSurfaceD3D()
    {
        if (TEXTURE != ZDrawSurface::TARGET::TEXTURE)
        {
            // Nothing allocated -> nothing released
            return;
        }

        D3D_SAFE_RELEASE(m_pColorSurface)
        D3D_SAFE_RELEASE(m_pDepthSurface)
        D3D_SAFE_RELEASE(m_pColorTexture)
        // Other fields are not deallocated here, idk why
    }

    void ZDrawSurfaceD3D::Begin()
    {
        if (m_lTarget != ZDrawSurface::TARGET::TEXTURE)
            return;
        
        g_pd3dDevice->GetRenderTarget(0, &m_pColorSurfaceOld);
        g_pd3dDevice->GetDepthStencilSurface(&m_pDepthSurfaceOld);
        g_pd3dDevice->GetViewport(&m_ViewportOld);
        g_pd3dDevice->SetRenderTarget(0, m_pColorSurface);
        g_pd3dDevice->SetDepthStencilSurface(m_pDepthSurface);
    }

    void ZDrawSurfaceD3D::End()
    {
        if (m_lTarget != ZDrawSurface::TARGET::TEXTURE)
            return;
        
        g_pd3dDevice->SetRenderTarget(0, m_pColorSurfaceOld);
        g_pd3dDevice->SetDepthStencilSurface(m_pDepthSurfaceOld);
        g_pd3dDevice->SetViewport(&m_ViewportOld);
    }

    void ZDrawSurfaceD3D::SetViewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
    {
        if (m_lTarget != ZDrawSurface::TARGET::TEXTURE)
            return;

        D3D_SAFE_RELEASE(m_pColorSurface);
        D3D_SAFE_RELEASE(m_pDepthSurface);
        D3D_SAFE_RELEASE(m_pColorTexture);

        g_pd3dDevice->CreateTexture(w, h, 1u, 1u, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pColorTexture, nullptr);
        m_pColorTexture->GetSurfaceLevel(0, &m_pColorSurface);

        // TODO: Finish me after ZRenderWintelD3D will be finished (format use from )
        // const D3DPRESENT_PARAMETERS* pPresentParams = &m_pRender->D3DPRESENT_PARAMETERS;
        // g_pd3dDevice->CreateDepthStencilSurface(w, h, pPresentParams->AutoDepthStencilFormat, D3DMULTISAMPLE_NONE, 0, true, &m_pDepthSurface, nullptr);
    }
}