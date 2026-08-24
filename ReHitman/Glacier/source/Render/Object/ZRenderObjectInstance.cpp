#include <Glacier/Render/Material/ZRenderMaterialInstance.h>
#include <Glacier/Render/Object/ZRenderObjectInstance.h>
#include <Glacier/Render/Object/ZRenderObject.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZRenderObjectInstance::ZRenderObjectInstance(ZRenderObject* pRenderObject, ZBaseGeom* pBaseGeom)
    {
        m_pRenderObject = pRenderObject;
        m_pBaseGeom = pBaseGeom;
        m_pRenderEntry = nullptr;
        ++pRenderObject->m_lNumActiveInstances;

        m_lSortValue = m_pRenderObject->m_pMaterialInstance->m_lRemapValue; // Rly?
        m_lFlags |= 1u;
    }

    ZRenderObjectInstance::~ZRenderObjectInstance()
    {
        ZSAFE_ASSERT(m_pRenderObject);
        ZSAFE_ASSERT(m_pRenderObject->m_lNumActiveInstances);

        if (!--m_pRenderObject->m_lNumActiveInstances && (m_pRenderObject->m_lFlags & 2))
        {
            ZUniMemory::Delete(m_pRenderObject);
        }
    }

    void ZRenderObjectInstance::UpdateLight(const SUpdateLightData* pUpdateLightData, uint32_t lNumLights)
    {
        // Do nothing
    }
}
