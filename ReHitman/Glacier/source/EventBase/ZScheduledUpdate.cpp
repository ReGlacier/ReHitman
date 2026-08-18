#include <G1ConfigurationService.h>
#include <Glacier/EventBase/ZScheduledUpdate.h>
#include <Glacier/EventBase/ZScheduledEvent.h>
#include <Glacier/EventBase/ZScheduledScript.h>
#include <Glacier/EventBase/ZEventBase.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/System/ZSysInterface.h>
#include <cassert>


namespace Glacier 
{
    extern bool s_bSleeperChanged;
    bool g_bBlockSchedUpdate = false;

    STATIC_CLASS_VAR_IMPL(ZScheduledUpdate, ZScheduledUpdate*, s_pScheduler, 0x008BE12C, nullptr);
    STATIC_CLASS_VAR_IMPL(ZScheduledUpdate, uint32_t, m_lMaxCyclesToRun, 0x007F5DD0, 1000000); // 1000000 - PC, 3000000 - PS2

    ZScheduledUpdate::ZScheduledUpdate()
        : m_lPos(0)
        , m_lPriority(0x100)
    {
        std::memset(m_lWaterJars, 0, sizeof(m_lWaterJars));
        ZScheduledUpdate::s_pScheduler = this;
    }

    ZScheduledUpdate::~ZScheduledUpdate()
    {
        // Kill running threads
        for (int i = 0; i < 16; ++i)
        {
            while (true)
            {
                auto* pFirst = m_pRunningThreads[i].First();
                if (!pFirst)
                {
                    break;
                }

                m_pRunningThreads[i].Remove();
                delete pFirst; // Because ZScheduledEvent has custom new/delete operators
            }
        }

        // Kill sleeping dogs
        while (true)
        {
            auto* pFirst = m_pSleepingThreads.First();
            if (!pFirst)
            {
                break;
            }

            m_pSleepingThreads.Remove();
            delete pFirst; // Because ZScheduledEvent has custom new/delete operators
        }

        ZScheduledUpdate::s_pScheduler = nullptr;
    }

    ZScheduledEvent* ZScheduledUpdate::AddEvent(ZEventBase* pEvent)
    {
        ZASSERT(pEvent->m_pScheduleEvent == nullptr);
        
        ZScheduledEvent* pResult = nullptr;
        if (!strcmp(pEvent->EventName(), "ScriptC")) // same to ScriptC (related to ZScriptC)
        {
            // Scripting stuff
            pResult = new ZScheduledScript(pEvent);
        }
        else
        {
            // Generic event stuff
            pResult = new ZScheduledEvent(pEvent);
        }

        pEvent->m_pScheduleEvent = pResult;
        m_pRunningThreads[0].Insert(pResult);

        return pResult;
    }

    void ZScheduledUpdate::RemoveEvent(ZEventBase* pEvent)
    {
        ZScheduledEvent* pSchedEvent = pEvent->m_pScheduleEvent;
        ZASSERT(pSchedEvent != nullptr);

        if (pSchedEvent != nullptr)
        {
            pSchedEvent->m_tNextRun = TIMETYPE(-2.0f);
            if (!pSchedEvent->m_bRunning)
            {
                pSchedEvent->m_bChanged = true;
            }

            pEvent->m_pScheduleEvent = nullptr;
        }
    }

    void ZScheduledUpdate::AttachEvent(ZScheduledEvent* pSchedEvent)
    {
        if (!pSchedEvent->m_bChanged)
        {
            return;
        }

        pSchedEvent->m_bChanged = 0;
        
        int lSec = static_cast<int>(pSchedEvent->m_tNextRun);
        if (lSec)
        {
            if (lSec <= 0)
            {
                if (lSec == -0x800)
                {
                    delete pSchedEvent; // Because ZScheduledEvent has custom new/delete operators
                }
            }
            else
            {
                TIMETYPE newTime = g_pSysInterface->m_fRealTime;
                m_pSleepingThreads.InsertSorted(pSchedEvent, newTime);
                ZASSERT(m_pSleepingThreads.Exists(pSchedEvent) == 1);
            }
        }
        else
        {
            const auto lPriority = pSchedEvent->GetPriority();
            m_pRunningThreads[lPriority].Insert(pSchedEvent);
            ZASSERT(m_pRunningThreads[pSchedEvent->m_lPriority].Exists(pSchedEvent) == 1);
        }
    }

    void ZScheduledUpdate::WakeSleepingThreads(TIMETYPE lTime)
    {
        while (true)
        {
            ZScheduledEvent* pFirstThread = m_pSleepingThreads.First();
            if (!pFirstThread)
                break;

            if (lTime < pFirstThread->m_tNextRun)
            {
                if (s_bSleeperChanged)
                {
                    s_bSleeperChanged = false;

                    ZScheduledEvent* pLastThread = m_pSleepingThreads.Last();
                    if (pLastThread)
                    {
                        ZScheduledEvent* pCurr = pLastThread;
                        do
                        {
                            ZScheduledEvent* pNext = pCurr->m_pNxt;

                            if (pCurr->m_bChanged)
                            {
                                m_pSleepingThreads.RemoveEntry(pCurr);

                                ZASSERT(m_pSleepingThreads.Exists(pCurr) == 0);
                                AttachEvent(pCurr);
                            }

                            pCurr = pNext;
                        } 
                        while (pCurr && pCurr != pLastThread);
                    }
                }
                return;
            }

            m_pSleepingThreads.Remove();
            pFirstThread->m_tNextRun = TIMETYPE(0);

            uint32_t priority = pFirstThread->m_lPriority;
            m_pRunningThreads[priority].Insert(pFirstThread);

            ZASSERT(m_pRunningThreads[priority].Exists(pFirstThread) == 1);
            ZASSERT(m_pSleepingThreads.Exists(pFirstThread) == 0);
        }
    }

    void ZScheduledUpdate::NextPriority()
    {
        ++m_lPos;

        if (m_lPos == 0x10)
        {
            m_lPriority = 0x100;
            m_lPos = 0;
        }
    }

    bool ZScheduledUpdate::DoesPriorityRun()
    {
        bool bResult = false;
        m_lWaterJars[m_lPos] += static_cast<uint16_t>(m_lPriority);
        m_lPriority >>= 1;

        if (m_lWaterJars[m_lPos] >= 0x100)
        {
            bResult = true;
            m_lWaterJars[m_lPos] -= 0x100;
        }

        NextPriority();
        return bResult;
    }

    void ZScheduledUpdate::ScheduleEvents()
    {
        if (g_bBlockSchedUpdate || g_pEngineData->IsPaused())
        {
            return;
        }

        TIMETYPE frameTime = g_pSysInterface->FrameTime;
        const int64_t startCycles = g_pSysInterface->TimeStampCounter(__FILE__, __LINE__);

        WakeSleepingThreads(frameTime);

        int emptyPriorityCount = 0;
        while (g_pSysInterface->TimeStampCounter(__FILE__, __LINE__) - startCycles < static_cast<uint64_t>(m_lMaxCyclesToRun))
        {
            const int currentPriority = m_lPos;
            ZScheduledEventList& runningThreads = m_pRunningThreads[currentPriority];
            ZScheduledEvent* pScheduledEvent = runningThreads.First();

            if (!pScheduledEvent)
            {
                if (++emptyPriorityCount == 16)
                {
                    return;
                }

                NextPriority();
                continue;
            }

            emptyPriorityCount = 0;

            if (!DoesPriorityRun())
            {
                continue;
            }

            pScheduledEvent->Run();

            const int nextRun = pScheduledEvent->m_tNextRun.secs;
            if (nextRun != 0)
            {
                runningThreads.Remove();

                if (nextRun > 0)
                {
                    m_pSleepingThreads.InsertSorted(pScheduledEvent, frameTime);
                }
                else if (nextRun == -0x800)
                {
                    delete pScheduledEvent; // Because ZScheduledEvent has custom new/delete operators
                }
            }
            else if (pScheduledEvent->m_lPriority != currentPriority)
            {
                runningThreads.Remove();
                m_pRunningThreads[pScheduledEvent->m_lPriority].Insert(pScheduledEvent);
            }

            runningThreads.MoveForward();
        }
    }

    void ZScheduledUpdate::SetEvent(ZEventBase* pEvent, ZScheduledEvent* pScheduledEvent)
    {
        pEvent->m_pScheduleEvent = pScheduledEvent;
    }
}
