#include <Glacier/Render/Entry/ZRenderEntrySprite.h>
#include <Glacier/Render/Entry/SRenderEntryInstance.h>
#include <Glacier/Render/Object/ZRenderObjectInstance.h>


namespace Glacier
{
    ZRenderEntrySprite::ZRenderEntrySprite(ZRenderEntryGeomCreateInfo* pInfo)
    {
        // TODO: Finish me
    }
    
    void ZRenderEntrySprite::SetLocal(bool bLocal)
    {
        m_bLocal = bLocal;
    }

    void ZRenderEntrySprite::SetSortValue(int32_t lSortValue)
    {
        // PC 0x00476E40
        if (!m_pRenderEntryInstances)
        {
            return;
        }

        for (uint16_t i = 0; i < m_lNumRenderEntryInstances; ++i)
        {
            auto* pRenderObjectInstance = m_pRenderEntryInstances[i]->pRenderObjectInstance;
            pRenderObjectInstance->m_lSortValue = (static_cast<uint32_t>(lSortValue) << 16) | static_cast<uint16_t>(pRenderObjectInstance->m_lSortValue);
        }
    }
}