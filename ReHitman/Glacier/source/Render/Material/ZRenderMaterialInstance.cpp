#include <Glacier/Render/Material/ZRenderMaterialBinderList.h>
#include <Glacier/Render/Material/ZRenderMaterialInstance.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZRenderMaterialInstance::ZRenderMaterialInstance(const char* pszName, ZRenderMaterialSubClass* pSubClass, SRMaterialProperties* pMatProperties, uint32_t lMaterialId)
        : m_pszName(pszName)
        , m_pMaterialSubClass(pSubClass)
        , m_pMaterialProperties(pMatProperties)
        , m_lMaterialId(lMaterialId)
        , m_lRemapValue(0u)
        , m_lLayerMask(0u)
    {
    }

    ZRenderMaterialInstance::~ZRenderMaterialInstance()
    {
        for (int i = 0; i < MAX_MATERIAL_BINDER_LISTS_NR; ++i)
        {
            if (m_pMaterialBinderList[i])
            {
                ZUniMemory::Delete(m_pMaterialBinderList[i]);
            }

            m_pMaterialBinderList[i] = nullptr;
        }
    }
    
    void ZRenderMaterialInstance::PrintInfo()
    {
        // Do nothing
    }
}