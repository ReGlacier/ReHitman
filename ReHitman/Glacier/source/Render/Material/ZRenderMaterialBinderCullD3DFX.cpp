#include <Glacier/Render/Material/ZRenderMaterialBinderCullD3DFX.h>
#include <Glacier/Render/Material/ZRenderMaterialEffectD3DFX.h>


namespace Glacier
{
    ZRenderMaterialBinderCullD3DFX::~ZRenderMaterialBinderCullD3DFX() = default;

    void ZRenderMaterialBinderCullD3DFX::Execute(const ZRenderContext* pContext)
    {
        D3DCULL dwCull = m_dwCull;

        // TODO: Finish this place after ZRender will be reversed
        // Expected decompiled call:
        //   if (dwCull == D3DCULL_CW && *reinterpret_cast<const uint8_t*>(reinterpret_cast<const char*>(pContext->m_pRender) + 0x133D))
        //       dwCull = D3DCULL_CCW;

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
