#include <Glacier/EventBase/ZScheduledEventList.h>
#include <Glacier/EventBase/ZScheduledUpdate.h>
#include <Glacier/EventBase/ZScheduledEvent.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    ZScheduledEventList::ZScheduledEventList() = default;

    ZScheduledEvent* ZScheduledEventList::First()
    {
        ZScheduledEvent* pLast = this->m_pLast;
    
        if (pLast)
        {
            ZScheduledEvent* pCurrent = pLast->m_pNxt;

            if (!pCurrent->m_bChanged)
            {
                return pCurrent;
            }
            
            while (true)
            {
                if (pLast == pCurrent)
                {
                    m_pLast = nullptr;
                }
                else
                {
                    pLast->m_pNxt = pCurrent->m_pNxt;
                }
                
                pCurrent->m_pNxt = nullptr;                
                ZScheduledUpdate::s_pScheduler->AttachEvent(pCurrent);
                
                pLast = m_pLast;
                if (!pLast)
                {
                    break;
                }
                
                pCurrent = pLast->m_pNxt;
                if (!pCurrent->m_bChanged)
                {
                    return pCurrent;
                }
            }
        }
        
        return nullptr;
    }

    ZScheduledEvent* ZScheduledEventList::Last() const
    {
        return m_pLast;
    }

    int ZScheduledEventList::Exists(ZScheduledEvent* pEvent) const
    {
        int count = 0;
        ZScheduledEvent* pCurrent = m_pLast;

        if (pCurrent)
        {
            do
            {
                if (pCurrent == pEvent)
                {
                    count++;
                }
                pCurrent = pCurrent->m_pNxt;
            } 
            while (pCurrent != m_pLast);
            
            return count;
        }
        
        return 0;
    }

    void ZScheduledEventList::Insert(ZScheduledEvent* pEvent)
    {
        ZScheduledEvent* pFirst = First(); // Head of circular buffer
    
        if (pFirst)
        {
            m_pLast->m_pNxt = pEvent;
            m_pLast = pEvent;
            m_pLast->m_pNxt = pFirst; // Close circular buffer
        }
        else
        {
            m_pLast = pEvent;
            m_pLast->m_pNxt = pEvent;
        }
    }

    void ZScheduledEventList::InsertSorted(ZScheduledEvent* pEvent, TIMETYPE timeDelta)
    {
        ZASSERT(m_pLast->m_pNxt->m_tNextRun <= m_pLast->m_tNextRun);

        pEvent->m_tNextRun.secs += timeDelta.secs; 
        TIMETYPE targetTime = pEvent->m_tNextRun;

        ZScheduledEvent* pCurrent = this->m_pLast;
        
        if (pCurrent)
        {
            do
            {
                if (pCurrent->m_pNxt->m_tNextRun.secs > targetTime.secs)
                {
                    pEvent->m_pNxt = pCurrent->m_pNxt;
                    pCurrent->m_pNxt = pEvent;
                    
                    ZASSERT(m_pLast->m_bChanged || m_pLast->m_pNxt->m_tNextRun <= m_pLast->m_tNextRun);
                    return;
                }
                
                pCurrent = pCurrent->m_pNxt;
            } 
            while (pCurrent != this->m_pLast);
        }

        Insert(pEvent);
        ZASSERT(m_pLast->m_bChanged || m_pLast->m_pNxt->m_tNextRun <= m_pLast->m_tNextRun);
    }
    
    void ZScheduledEventList::Remove()
    {
        ZASSERT(Last());
        ZScheduledEvent* pEvent = First();
        ZASSERT(pEvent != nullptr);

        ZScheduledEvent* pLast = Last();

        if (pLast == pEvent)
        {
            m_pLast = nullptr;
        }
        else
        {
            pLast->m_pNxt = pEvent->m_pNxt;
        }

        pEvent->m_pNxt = nullptr;
    }

    void ZScheduledEventList::RemoveEntry(ZScheduledEvent* pPrevEvent)
    {
        ZScheduledEvent* pTarget = pPrevEvent->m_pNxt; 

        if (pTarget == pPrevEvent)
        {
            pPrevEvent->m_pNxt = nullptr;
            m_pLast = nullptr;
        }
        else
        {
            if (pTarget == m_pLast)
            {
                m_pLast = pPrevEvent;
            }
            
            pPrevEvent->m_pNxt = pTarget->m_pNxt;
            pTarget->m_pNxt = nullptr;
        }

        ZASSERT(!m_pLast || m_pLast->m_bChanged || m_pLast->m_pNxt->m_tNextRun <= m_pLast->m_tNextRun);
    }

    void ZScheduledEventList::MoveForward()
    {
        ZScheduledEvent* pFirst = First();
        if (pFirst)
        {
            m_pLast = pFirst;
        }
    }
}