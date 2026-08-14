#include <Glacier/Render/Material/ZRenderMaterialResourceD3DFX.h>
#include <Glacier/Render/Material/ZRenderMaterialLayerD3DFX.h>
#include <Glacier/Render/ZRPropertyReader.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZSTL/MYSTR.h>


namespace Glacier
{
    ZRenderMaterialResourceD3DFX::ZRenderMaterialResourceD3DFX()
        : ZRenderMaterialResourceD3D()
    {
        // TODO: Finish me
    }

    ZRenderMaterialResourceD3DFX::~ZRenderMaterialResourceD3DFX()
    {
        FreeResources();
    }
    
    void ZRenderMaterialResourceD3DFX::GetShaderPath(zstring& sPath)
    {
        MYSTR sComputedPath = g_pSysInterface->ProjectPath() + MYSTR("Shaders\\PC\\FX");
        sPath = sComputedPath.String;
    }

    void ZRenderMaterialResourceD3DFX::CreateEffect(const char* pszFileName, const D3DXMACRO* pDefined, uint32_t lNumDefines)
    {
        // TODO: Finish me
    }

    void ZRenderMaterialResourceD3DFX::AddEffect(const char* pszFileName, ZRenderMaterialSubClassD3D* pMaterialSubClass)
    {
        // TODO: Finish me
    }

    ZRenderMaterialLayerD3DFX* ZRenderMaterialResourceD3DFX::AddLayer(ZRenderMaterialLayerD3DFX* pLayer)
    {
        for (uint32_t i = 0; i < m_lNumLayers; ++i)
        {
            if (m_pMaterialLayers[i]->m_hTechnique == pLayer->m_hTechnique)
            {
                // delete layer because layer with current technique already exists
                ZUniMemory::Delete(pLayer);

                return m_pMaterialLayers[i];
            }
        }

        // Check for out of bounds and store at new index
        ZASSERT(m_lNumLayers < MAX_NUM_FX_MATERIAL_LAYERS);
        m_pMaterialLayers[m_lNumLayers++] = pLayer;

        return pLayer;
    }

    void ZRenderMaterialResourceD3DFX::CreateMaterialLayer(ZRenderMaterialSubClassD3D* pMaterialSubClass, const ZRPropertyReader* pMatPropReader)
    {
        // TODO: FInish me
    }

    void ZRenderMaterialResourceD3DFX::FreeResources()
    {
        // TODO: Finish me
    }
}