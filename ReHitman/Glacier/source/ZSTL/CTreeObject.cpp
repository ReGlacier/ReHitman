#include <Glacier/ZSTL/CTreeObject.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    void CTreeObjectList::Attach(CTreeObject* pObject)
    {
        ZASSERT(pObject != nullptr);

        if (m_pHead)
        {
            m_pHead->m_pPrev = pObject;
        }

        pObject->m_pNext = m_pHead;
        pObject->m_pPrev = nullptr;
        m_pHead = pObject;
    }

    void CTreeObjectList::Detach(CTreeObject* pObject)
    {
        if (pObject->m_pPrev)
        {
            pObject->m_pPrev->m_pNext = pObject->m_pNext;
        }
        else
        {
            m_pHead = pObject->m_pNext;
        }

        if (pObject->m_pNext)
        {
            pObject->m_pNext->m_pPrev = pObject->m_pPrev;
        }
        
        pObject->m_pNext = nullptr;
        pObject->m_pPrev = nullptr;
    }

    CTreeObject* CTreeObjectList::GetHead() const
    {
        return m_pHead;
    }
}