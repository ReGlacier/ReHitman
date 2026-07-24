#include <Glacier/EventBase/ScriptCoroutineScheduler.h>
#include <Glacier/EventBase/ZScheduledEvent.h>
#include <Glacier/EventBase/ZScheduledUpdate.h>
#include <Glacier/EventBase/ZEventBase.h>
#include <Glacier/ZSysInterface.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    bool s_bSleeperChanged { false };
    bool s_bFreedPriorities[ZScheduledEvent::MAX_PRIORITIES_NR] { false };

    void* ZScheduledEvent::operator new(std::size_t size)
    {
        return ZScheduledEvent::s_PoolAllocator.Alloc(sizeof(ZScheduledEvent));
    }
    
    void ZScheduledEvent::operator delete(void* ptr)
    {
        ZScheduledEvent::s_PoolAllocator.Free(ptr);
    }

    ZScheduledEvent::ZScheduledEvent(ZEventBase* pEvent)
        : m_rEvent(pEvent->GetRef())
        , m_pNxt(nullptr)
        , m_pThread(nullptr)
        , m_pStoredStack(nullptr)
        , m_bChanged(0)
        , m_bRunning(0)
        , m_lStoredStackLength(0)
        , m_lPriority(0)
        , m_tNextRun(0)
    {
    }

    void ZScheduledEvent::Run()
    {
        m_bRunning = 1;

        s_rEvent = m_rEvent;
        g_pFunction = ZScheduledEvent::EnterFunction;

        g_pStoredStack = m_pStoredStack;
        g_lStoredStackLength = m_lStoredStackLength & ~3u;

        EnterSchedulerMode();

        m_pStoredStack = static_cast<char*>(g_pStoredStack);
        m_lStoredStackLength = g_lStoredStackLength & ~3u;

        s_rEvent = 0;
        g_pFunction = nullptr;
    }

    ZScheduledEvent* ZScheduledEvent::GetNextThread()
    {
        return m_pThread;
    }

    ZScheduledEvent* ZScheduledEvent::Fork()
    {
        auto* pForked = new ZScheduledEvent(ZEventBase::RefToPtr(m_rEvent));
        return Fork(pForked);
    }

    ZScheduledEvent* ZScheduledEvent::Fork(ZScheduledEvent* pEvent)
    {
        ZScheduledEvent* pThread = m_pThread;
        if (!pThread)
        {
            pThread = this;
        }
        pEvent->m_pThread = pThread;
        m_pThread = pEvent;

        pEvent->m_lPriority = m_lPriority;
        pEvent->m_bChanged = 1;

        ZScheduledUpdate::s_pScheduler->m_pRunningThreads[m_lPriority].Insert(pEvent);

        return pEvent;
    }
    
    const uint32_t ZScheduledEvent::SetPriority(uint32_t lPriority)
    {
        ZASSERT(lPriority < MAX_PRIORITIES_NR);

        const uint32_t lOldPriority = m_lPriority;
        m_lPriority = lPriority;

        if (!m_bRunning && (m_tNextRun == 0))
        {
            m_bChanged = 1;
            s_bSleeperChanged = true;
            s_bFreedPriorities[lOldPriority] = 1;
        }

        return lOldPriority;
    }

    const uint32_t ZScheduledEvent::GetPriority() const
    {
        return m_lPriority;
    }

    const float ZScheduledEvent::GetSleepTimeLeft() const
    {
        const float fTimeToRun = m_tNextRun - g_pSysInterface->m_fRealTime;
        if (fTimeToRun <= 0.0f)
        {
            return 0.0f;
        }

        return fTimeToRun;
    }

    bool ZScheduledEvent::HasThreads() const
    {
        return m_pThread != nullptr;
    }

    void ZScheduledEvent::EnterFunction()
    {
        auto* pEvent = ZEventBase::RefToPtr(ZScheduledEvent::s_rEvent);
        ZASSERT(pEvent != nullptr);

        // I'm not sure about this, but looks legit
        pEvent->SchedUpdate();
    }

    ZREF ZScheduledEvent::s_rEvent { 0 };
    uint8_t ZScheduledEvent::s_PoolAllocatorBuffer[0x4000] { 0 };
    ZPoolAllocator ZScheduledEvent::s_PoolAllocator { (char*)&ZScheduledEvent::s_PoolAllocatorBuffer[0], 0x4000u, "ZScheduledEvent::s_PoolAllocator", false };
}
