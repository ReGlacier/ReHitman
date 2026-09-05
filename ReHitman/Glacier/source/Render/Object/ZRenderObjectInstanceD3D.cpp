#include <Glacier/Render/Object/ZRenderObjectInstanceD3D.h>
#include <Glacier/Render/ZDirect3DDevice.h>
#include <Glacier/Render/ZRIndexContainer.h>
#include <Glacier/Render/Globals.h>


namespace Glacier
{
    void ZRenderObjectInstanceD3D::DrawIndexedTriangles(const ZRIndexContainer* pIndexContainer, uint32_t lNumIndices, ZRenderWintelD3D* pRender, uint32_t lObjectSubType)
    {
        for (int i = 0; i < pIndexContainer->m_lSubRangeCount; ++i)
        {
            g_pd3dDevice->DrawIndexedPrimitive(
                D3DPT_TRIANGLELIST, 
                0, 
                0, 
                lNumIndices, 
                pIndexContainer->m_pSubRanges[i].m_lIndexCount, 
                pIndexContainer->m_pSubRanges[i].m_lIndexCount / 3);
        }
    }

    void ZRenderObjectInstanceD3D::DrawIndexedStrips(const ZRIndexContainer* pIndexContainer, uint32_t lNumIndices, ZRenderWintelD3D* pRender, uint32_t lObjectSubType)
    {
        for (int i = 0; i < pIndexContainer->m_lSubRangeCount; ++i)
        {
            g_pd3dDevice->DrawIndexedPrimitive(
                D3DPT_TRIANGLESTRIP, 
                0, 
                0, 
                lNumIndices, 
                pIndexContainer->m_pSubRanges[i].m_lIndexOffset, 
                pIndexContainer->m_pSubRanges[i].m_lIndexCount - 2);
        }
        
    }
}