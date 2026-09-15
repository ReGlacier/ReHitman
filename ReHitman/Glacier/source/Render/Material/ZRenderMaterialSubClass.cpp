#include <Glacier/Render/Material/ZRenderMaterialSubClass.h>
#include <Glacier/Render/Material/ZRenderMaterialInstance.h>
#include <Glacier/Render/Material/ZRenderMaterialLayer.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    ZRenderMaterialSubClass::ZRenderMaterialSubClass(const char* pszName, ZRenderMaterialClass* pMaterialClass, uint16_t lObjectType, uint16_t lObjectSubType, uint32_t lMaterialFlags)
    {
        m_lMaterialFlags = lMaterialFlags;
        m_pszName = pszName;
        m_pMaterialClass = pMaterialClass;
        m_lObjectType = lObjectType;
        m_lObjectSubType = lObjectSubType;
        m_lIndex = 0u;
        m_pLayers[0] = nullptr;
        m_pLayers[1] = nullptr;
        m_pLayers[2] = nullptr;
        m_pLayers[3] = nullptr;
        m_pLayers[4] = nullptr;
        m_pLayers[5] = nullptr;
        m_pLayers[6] = nullptr;
        m_pLayers[7] = nullptr;
        m_pLayers[8] = nullptr;
        m_pLayers[9] = nullptr;
        m_pLayers[10] = nullptr;
        m_pLayers[11] = nullptr;
        m_pLayers[12] = nullptr;
        m_pLayers[13] = nullptr;
        m_pLayers[14] = nullptr;
        m_pLayers[15] = nullptr;
        m_pLayers[16] = nullptr;
        m_pLayers[17] = nullptr;
    }

    ZRenderMaterialSubClass::~ZRenderMaterialSubClass() = default;

    void ZRenderMaterialSubClass::Initialize()
    {
        for (int i = MAX_LAYERS_NR - 1; i != -1; --i)
        {
            if (m_pLayers[i])
            {
                m_pLayers[i]->Initialize();
            }
        }
    }

    uint32_t ZRenderMaterialSubClass::Begin(uint32_t lLayerIndex, const ZRenderContext* pCtx)
    {
        if (!m_pLayers[lLayerIndex])
        {
            return 0u;
        }

        m_lLayer = lLayerIndex;
        return m_pLayers[lLayerIndex]->BeginSubClass(this, pCtx);
    }

    void ZRenderMaterialSubClass::End()
    {
        ZASSERT(m_pLayers[m_lLayer]);
        m_pLayers[m_lLayer]->EndSubClass();
    }

    void ZRenderMaterialSubClass::BeginPass(uint32_t lPassIndex, const ZRenderContext* pCtx)
    {
        m_lPass = lPassIndex;
        m_pLayers[m_lLayer]->BeginPass(lPassIndex, pCtx);
    }

    void ZRenderMaterialSubClass::EndPass()
    {
        m_pLayers[m_lLayer]->EndPass();
    }

    void ZRenderMaterialSubClass::BeginInstance(const ZRenderMaterialInstance* pMaterial, const ZRenderContext* pCtx)
    {
        m_pLayers[m_lLayer]->BeginInstance(pMaterial, pMaterial->m_pMaterialBinderList[m_lLayer], pCtx);
    }

    void ZRenderMaterialSubClass::EndInstance()
    {
        m_pLayers[m_lLayer]->EndInstance();
    }

    ZPrimAccess* ZRenderMaterialSubClass::CreatePrimAccess(const ZPrimHandle& hPrim)
    {
        return nullptr;
    }

    uint32_t ZRenderMaterialSubClass::GetPrimAccessSize(const ZPrimHandle& hPrim)
    {
        return 0u;
    }

    void ZRenderMaterialSubClass::CreatePrimAccessInplace(ZPrimAccess* pWhere, const ZPrimHandle& hPrim)
    {
        // Do nothing
    }
}