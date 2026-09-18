#include <Glacier/Render/Material/ZRenderMaterialBinderIntD3DFX.h>
#include <Glacier/Render/Material/ZRenderMaterialEffectD3DFX.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    ZRenderMaterialBinderIntD3DFX::~ZRenderMaterialBinderIntD3DFX() = default;

    ZRenderMaterialBinderIntD3DFX::ZRenderMaterialBinderIntD3DFX(const char* pszName, uint32_t* pInts, uint32_t lNumInts, ZRenderMaterialEffectD3DFX* pEffect, D3DXHANDLE hInts)
    {
        m_pszName = pszName;
        m_hInts = hInts;
        m_lBinderType = 1;
        m_lNumInts = lNumInts;
        m_pEffect = pEffect;

        if (lNumInts)
        {
            ZASSERT(lNumInts <= 4);

            for (int i = 0; i < lNumInts; ++i)
            {
                m_Ints[i] = pInts[i];
            }
        }
    }

    void ZRenderMaterialBinderIntD3DFX::Execute(const ZRenderContext* pContext)
    {
        ID3DXEffect* pD3DXEffect = m_pEffect->m_pD3DXEffect;

        if (m_lNumInts == 1)
        {
            pD3DXEffect->SetInt(m_hInts, m_Ints[0]);
        }
        else
        {
            pD3DXEffect->SetIntArray(m_hInts, m_Ints, m_lNumInts);
        }
    }

    int32_t* ZRenderMaterialBinderIntD3DFX::GetValues()
    {
        return &m_Ints[0];
    }
}
