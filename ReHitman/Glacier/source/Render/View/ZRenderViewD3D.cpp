#include <Glacier/Render/PostFilter/ZPostFilterWintelD3D.h>
#include <Glacier/Render/Draw/ZDrawSurfaceD3D.h>
#include <Glacier/Render/View/ZRenderViewD3D.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    void ZRenderViewD3D::CreateSurface(ZDrawSurface::TARGET lTarget)
    {
        m_bOwnerDraw = (lTarget == ZDrawSurface::TARGET::TEXTURE);
        m_pDrawSurface = ZUniMemory::New<ZDrawSurfaceD3D>(lTarget, reinterpret_cast<ZRenderWintelD3D*>(Render()));
    }

    void ZRenderViewD3D::CreatePostFilter()
    {
        if (!m_bPostfilterEnabled) return;
    
        ZASSERT(!m_pPostFilter);
        if (g_pPostFilter)
        {
            g_pPostFilter->FreeDeviceBuffers();
        }

        auto* pPostFilter = ZUniMemory::New<ZPostFilterWintelD3D>();
        m_pPostFilter = pPostFilter;
        
        pPostFilter->Init();
        pPostFilter->SetViewport(
            m_lViewPort[0], 
            m_lViewPort[1], 
            m_lViewPort[2] - m_lViewPort[0], 
            m_lViewPort[3] - m_lViewPort[1]
        );

        g_pPostFilter = pPostFilter;
    }
}