#include <Glacier/Render/Material/ZRenderMaterialBinderParserD3DFX.h>
#include <Glacier/Render/Material/ZRenderMaterialLayerD3DFX.h>
#include <Glacier/Render/Material/ZRenderMaterialEffectD3DFX.h>
#include <Glacier/Render/Material/ZRenderMaterialBinderList.h>
#include <Glacier/Render/ZRenderBinderList.h>


namespace Glacier
{
    ZRenderMaterialLayerD3DFX::ZRenderMaterialLayerD3DFX(const char* pszName, D3DXHANDLE hTechnique, ZRenderMaterialEffectD3DFX* pEffect)
        : ZRenderMaterialLayer(pszName)
        , m_pEffect(pEffect)
        , m_hTechnique(hTechnique)
    {
        if (m_hTechnique && m_pEffect && m_pEffect->m_pD3DXEffect)
        {
            m_pEffect->m_pD3DXEffect->GetTechniqueDesc(m_hTechnique, &m_TechniqueDesc);
        }
    }

    ZRenderMaterialLayerD3DFX::~ZRenderMaterialLayerD3DFX() = default;

    uint32_t ZRenderMaterialLayerD3DFX::BeginSubClass(const ZRenderMaterialSubClass* pSubClass, const ZRenderContext* pContext)
    {
        if (!m_hTechnique || !m_pEffect || !m_pEffect->m_pD3DXEffect)
        {
            return 0;
        }

        m_pEffect->m_pD3DXEffect->SetTechnique(m_hTechnique);

        UINT uPasses = 0;
        m_pEffect->m_pD3DXEffect->Begin(
            &uPasses, 
            D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESHADERSTATE | D3DXFX_DONOTSAVESAMPLERSTATE // 7
        );

        return uPasses;
    }

    void ZRenderMaterialLayerD3DFX::EndSubClass()
    {
        if (m_pEffect && m_pEffect->m_pD3DXEffect)
        {
            m_pEffect->m_pD3DXEffect->End();
        }
    }

    void ZRenderMaterialLayerD3DFX::BeginPass(uint32_t lPassIndex, const ZRenderContext* pContext)
    {
        m_pEffect->m_pD3DXEffect->BeginPass(lPassIndex);
    }

    void ZRenderMaterialLayerD3DFX::EndPass()
    {
        if (m_pEffect && m_pEffect->m_pD3DXEffect)
        {
            m_pEffect->m_pD3DXEffect->EndPass();
        }
    }

    void ZRenderMaterialLayerD3DFX::BeginInstance(const ZRenderMaterialInstance* pMaterialInstance, const ZRenderMaterialBinderList* pBinderList, const ZRenderContext* pContext)
    {
        pBinderList->m_BindersMaterial.ExecuteBinders(pContext);
    }

    void ZRenderMaterialLayerD3DFX::EndInstance()
    {
        // Do nothing
    }

    void ZRenderMaterialLayerD3DFX::BeginObject(
        const ZRenderMaterialInstance* pMaterialInstance, 
        const ZRenderMaterialBinderList* pBinderList, 
        const ZRenderObjectInstance* pInstance, 
        const ZRenderContext* pContext
    )
    {
        pBinderList->m_BindersObject.ExecuteBinders(pContext);

        if (m_pEffect && m_pEffect->m_pD3DXEffect)
        {
            m_pEffect->m_pD3DXEffect->CommitChanges();
        }
    }

    void ZRenderMaterialLayerD3DFX::EndObject()
    {
        // Do nothing
    }

    uint32_t ZRenderMaterialLayerD3DFX::NumPasses()
    {
        return m_hTechnique ? m_TechniqueDesc.Passes : 0u;
    }

    void ZRenderMaterialLayerD3DFX::CreateBinders(int nParam, ZRenderMaterialInstance* pMaterialInstance, ZRenderMaterialBinderList* pBinderList)
    {
        if (!m_hTechnique) return;

        ZRenderMaterialBinderParserD3DFX parser(m_pEffect, m_hTechnique);
        parser.CreateBinders(pBinderList, pMaterialInstance);
    }
}