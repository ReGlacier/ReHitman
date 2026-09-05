#include <Glacier/Component/ZComponentManagerBase.h>


namespace Glacier
{
    ZComponentManagerBase::ZComponentManagerBase() = default;
    ZComponentManagerBase::~ZComponentManagerBase() = default;

    void ZComponentManagerBase::Add(ZComponentBase* pComponent)
    {
        if (m_pLast)
        {
            m_pLast->m_pNext = pComponent;
            pComponent->m_pPrev = m_pLast;
            m_pLast = pComponent;
        }
        else
        {
            m_pFirst = pComponent;
            m_pLast = pComponent;
        }
    }

    ZComponentBase* ZComponentManagerBase::Begin(CallDirection eDirection)
    {
        return eDirection == BACK ? m_pLast : m_pFirst;
    }

    ZComponentBase* ZComponentManagerBase::Next(ZComponentBase* pComponent, CallDirection eDirection)
    {
        return eDirection == BACK ? pComponent->m_pPrev : pComponent->m_pNext;
    }
}
