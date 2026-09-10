#include <Glacier/Render/Material/ZRenderMaterialBinderCullD3DFX.h>
#include <Glacier/Render/Material/ZRenderMaterialEffectD3DFX.h>
#include <Glacier/Render/ZRenderContext.h>
#include <Glacier/Render/ZRenderX86.h>


namespace Glacier
{
    ZRenderMaterialBinderCullD3DFX::~ZRenderMaterialBinderCullD3DFX() = default;

    void ZRenderMaterialBinderCullD3DFX::Execute(const ZRenderContext* pContext)
    {
        D3DCULL dwCull = m_dwCull;

        if (dwCull == D3DCULL_CW && static_cast<const ZRenderX86*>(pContext->m_pRender)->m_bCullingReversed)
        {
            dwCull = D3DCULL_CCW;
        }

        m_pEffect->m_pD3DXEffect->SetInt(m_hCull, dwCull);
    }

    int32_t* ZRenderMaterialBinderCullD3DFX::GetValues()
    {
        return reinterpret_cast<int32_t*>(&m_dwCull);
    }

    ZRenderMaterialBinderCullD3DFX::ZRenderMaterialBinderCullD3DFX(const char *pszName, D3DCULL dwCull, ZRenderMaterialEffectD3DFX *pEffect, D3DXHANDLE hCull)
    {
        m_lBinderType = 1;
        m_pszName = pszName;
        m_dwCull = dwCull;
        m_pEffect = pEffect;
        m_hCull = hCull;
    }
}
