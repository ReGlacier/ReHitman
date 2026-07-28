#include <Glacier/EventBase/ZEventList.h>
#include <Glacier/EventBase/ZEventBase.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    ZEventList::ZEventList()
        : m_NumberOfEvents(0u)
        , m_Iterator(reinterpret_cast<ZListNode<ZEventBase, 0>*>(&m_EventLists[9]))
        , m_pCurrentEvent(nullptr)
    {
    }

    ZEventList::~ZEventList() = default;

    void ZEventList::DoInit()
    {
        const uint32_t NumberOfEvents = m_NumberOfEvents;
        uint32_t* EventRefs = reinterpret_cast<uint32_t*>(alloca(sizeof(uint32_t) * NumberOfEvents));
        uint32_t* pCurrentEventRef = &EventRefs[0];

        for (int i = 9; i != -1; --i)
        {
            auto it = m_EventLists[i].Begin();

            while (it != m_EventLists[i].End())
            {
                *pCurrentEventRef = it->GetRef();
                ++pCurrentEventRef;
                ++it;
            }
        }

        ZASSERT(pCurrentEventRef - EventRefs == NumberOfEvents);
        for (int i = 0; i < NumberOfEvents; ++i)
        {
            auto* pCurrentEvent = ZEventBase::RefToPtr(EventRefs[i]);
            if (pCurrentEvent)
            {
                ZASSERT(pCurrentEvent->m_pBaseGeom);
                pCurrentEvent->DoInit();
            }
        }
    }

    void ZEventList::Clear()
    {
        for (int i = 0; i < 10; ++i)
        {
            m_EventLists[i].UnlinkAll();
        }

        m_NumberOfEvents = 0u;
    }

    void ZEventList::FrameUpdate()
    {
        g_pSysInterface->m_fMainCurTime.secs = g_pSysInterface->FrameTime.secs;

        ZEngineDataBase* pEngineData = g_pSysInterface->m_pEngineData;
        const bool bPaused = pEngineData->m_bPause;
        ZEventBase* pOnlyEventUpdate = pEngineData->GetOnlyEventUpdate();

        for (int i = 0; i < ZEventBase::INACTIVE_LIST; ++i)
        {
            m_Iterator = m_EventLists[i].Begin();

            while (m_Iterator != m_EventLists[i].End())
            {
                ZListIterator<ZEventBase, 0> current = m_Iterator++;
                m_pCurrentEvent = static_cast<ZEventBase*>(current);

                ZASSERT((m_pCurrentEvent->m_lEventLists & (EV_FRMUPD | EV_TIMEUPD)) != 0);

                if (bPaused && (m_pCurrentEvent->m_lRoutCases & 0x100u) == 0)
                {
                    continue;
                }

                if (pOnlyEventUpdate != nullptr && pOnlyEventUpdate != m_pCurrentEvent)
                {
                    continue;
                }

                ZGEOM* pGeom = m_pCurrentEvent->m_pBaseGeom;
                ZASSERT(pGeom);
                ZASSERT(pGeom->Active());
                ZASSERT(pGeom->IsInitialized());

                if ((m_pCurrentEvent->m_lEventLists & EV_TIMEUPD) != 0)
                {
                    if (m_pCurrentEvent->m_ClassCall)
                    {
                        pGeom->ClassFrameUpdate();
                    }
                    else
                    {
                        m_pCurrentEvent->Call(0x10u, nullptr, 0);
                    }
                }

                ZEventBase* pCurrentEvent = m_pCurrentEvent;
                if (pCurrentEvent && (pCurrentEvent->m_lEventLists & EV_FRMUPD) != 0 && g_pSysInterface->FrameTime >= pCurrentEvent->m_fTimePassed)
                {
                    pCurrentEvent->m_fTimePassed.secs += static_cast<int>(pCurrentEvent->m_TimerInterval * TIMETYPE::kTicksPerSecond);

                    if (pCurrentEvent->m_ClassCall)
                    {
                        pGeom->ClassTimeUpdate();
                    }
                    else
                    {
                        pCurrentEvent->Call(0x8u, nullptr, 0);
                    }
                }
            }
        }

        m_pCurrentEvent = nullptr;
    }

    void ZEventList::AddEvent(ZEventBase* event)
    {
        ZASSERT(event->m_pBaseGeom==0 || event->m_pBaseGeom->Active());
        ++m_NumberOfEvents;

        ZListNode<ZEventBase, 0>* pToInsert = nullptr;
        if (event)
        {
            pToInsert = static_cast<ZListNode<ZEventBase, 0>*>(event);
        }

        m_EventLists[9].AddLast(pToInsert);
    }

    void ZEventList::RemoveEvent(ZEventBase* pEvent)
    {
        if (ZEventBase* pLast = static_cast<ZEventBase*>(m_Iterator); pLast == pEvent)
        {
            ++m_Iterator;
        }

        if (pEvent == m_pCurrentEvent)
        {
            m_pCurrentEvent = nullptr;
        }

        --m_NumberOfEvents;
        static_cast<ZListNode<ZEventBase,0>*>(pEvent)->Unlink();
    }

    void ZEventList::ActivateFrameUpdate(ZEventBase* pEvent)
    {
        auto* pNode = pEvent ? static_cast<ZListNode<ZEventBase, 0>*>(pEvent) : nullptr;
        if (pNode)
            pNode->Unlink();

        m_EventLists[pEvent->GetEventPriority()].AddFirst(pNode);
    }

    void ZEventList::DeactivateFrameUpdate(ZEventBase* pEvent)
    {
        if (pEvent == static_cast<ZEventBase*>(m_Iterator))
            ++m_Iterator;

        if (pEvent == m_pCurrentEvent)
            m_pCurrentEvent = nullptr;

        auto* pNode = pEvent ? static_cast<ZListNode<ZEventBase, 0>*>(pEvent) : nullptr;
        if (pNode)
            pNode->Unlink();

        m_EventLists[ZEventBase::INACTIVE_LIST].AddLast(pNode);
    }
}
