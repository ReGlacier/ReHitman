#include <Glacier/Render/Material/ZRenderMaterialBinderFloatD3DFX.h>
#include <Glacier/Render/Material/ZRenderMaterialEffectD3DFX.h>


namespace Glacier
{
    ZRenderMaterialBinderFloatD3DFX::ZRenderMaterialBinderFloatD3DFX(const char* pszName, const float* pFloats, uint32_t lNumFloats, ZRenderMaterialEffectD3DFX* pEffect, D3DXHANDLE hFloats)
    {
        m_pszName = pszName;
        m_hFloats = hFloats;
        m_pFloats = pFloats;
        m_lBinderType = 0;
        m_lNumFloats = lNumFloats;
        m_pEffect = pEffect;
    }

    ZRenderMaterialBinderFloatD3DFX::~ZRenderMaterialBinderFloatD3DFX() = default;

    void ZRenderMaterialBinderFloatD3DFX::Execute(const ZRenderContext* pContext)
    {
        ID3DXEffect* pD3DXEffect = m_pEffect->m_pD3DXEffect;

        if (m_lNumFloats == 4)
        {
            // vec4
            pD3DXEffect->SetVector(m_hFloats, reinterpret_cast<const D3DXVECTOR4*>(m_pFloats));
        }
        else if (m_lNumFloats == 16)
        {
            // mat4x4
            pD3DXEffect->SetMatrix(m_hFloats, reinterpret_cast<const D3DXMATRIX*>(m_pFloats));
        }
        else
        {
            // float[]
            pD3DXEffect->SetFloatArray(m_hFloats, m_pFloats, m_lNumFloats);
        }
    }

    const float* ZRenderMaterialBinderFloatD3DFX::GetValues() const
    {
        return m_pFloats;
    }
}
