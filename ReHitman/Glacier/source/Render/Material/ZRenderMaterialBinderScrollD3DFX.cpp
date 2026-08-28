#include <Glacier/Render/Material/ZRenderMaterialBinderScrollD3DFX.h>
#include <Glacier/Render/Material/ZRenderMaterialEffectD3DFX.h>
#include <Glacier/System/ZSysInterface.h>
#include <cmath>


namespace Glacier
{
    ZRenderMaterialBinderScrollD3DFX::ZRenderMaterialBinderScrollD3DFX(ZRenderMaterialEffectD3DFX* pEffect, D3DXHANDLE hScrollPosition, const float* pfScrollSpeed)
    {
        m_pszName = "gm_vScrollPosition";
        m_pEffect = pEffect;
        m_lBinderType = 5;
        m_hScrollPosition = hScrollPosition;
        m_fScrollSpeed[0] = pfScrollSpeed[0];
        m_fScrollSpeed[1] = pfScrollSpeed[1];
        m_fScrollSpeed[2] = pfScrollSpeed[2];
        m_fScrollSpeed[3] = pfScrollSpeed[3];
    }

    ZRenderMaterialBinderScrollD3DFX::~ZRenderMaterialBinderScrollD3DFX() = default;

    void ZRenderMaterialBinderScrollD3DFX::Execute(const ZRenderContext* pContext)
    {
        // PC 00496850: scroll UV by game time, wrapped into [0, 1)
        const float fFrameTime = static_cast<float>(static_cast<double>(g_pSysInterface->FrameTime.secs) * 0.0009765625);
        const float aScrollPosition[4] =
        {
            fmodf(fFrameTime * m_fScrollSpeed[0], 1.0f),
            fmodf(fFrameTime * m_fScrollSpeed[1], 1.0f),
            0.0f,
            0.0f,
        };

        m_pEffect->m_pD3DXEffect->SetFloatArray(m_hScrollPosition, aScrollPosition, 4);
    }
}
