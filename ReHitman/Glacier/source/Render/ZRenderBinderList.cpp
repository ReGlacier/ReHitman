#include <Glacier/Render/ZRenderBinderList.h>
#include <Glacier/Render/ZRenderBinder.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZRenderBinderList::~ZRenderBinderList()
    {
        for (int i = 0; i < m_lNumBinders; ++i)
        {
            if (m_pBinders[i])
            {
                ZUniMemory::Delete(m_pBinders[i]);
                m_pBinders[i] = nullptr;
            }
        }

        if (m_pBinders)
        {
            ZUniMemory::Free(m_pBinders);
        }

        m_pBinders = nullptr;
    }

    void ZRenderBinderList::ExecuteBinders(const ZRenderContext* pContext) const
    {
        if (m_lNumBinders == 0u || !m_pBinders)
            return;

        for (uint32_t i = 0; i < m_lNumBinders; ++i)
        {
            if (!m_pBinders[i])
                continue;

            m_pBinders[i]->Execute(pContext);
        }
    }
}