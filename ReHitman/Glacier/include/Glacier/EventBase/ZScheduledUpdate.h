#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/EventBase/ZScheduledEventList.h>
#include <Glacier/ZSTL/TIMETYPE.h>
#include <Glacier/ZUniMemory.h>
#include <cstdint>


namespace Glacier 
{
    // fwds
    class ZScheduledEvent;
    class ZEventBase;

    class ZScheduledUpdate 
    {
    public:
        // constant
        // static
        STATIC_CLASS_VAR(ZScheduledUpdate, ZScheduledUpdate*, s_pScheduler);
        STATIC_CLASS_VAR(ZScheduledUpdate, uint32_t, m_lMaxCyclesToRun);

        // vtbl
        // methods
        ZScheduledUpdate();
        ~ZScheduledUpdate();
        ZScheduledEvent* AddEvent(ZEventBase* pEvent);
        void RemoveEvent(ZEventBase* pEvent);
        void AttachEvent(ZScheduledEvent* pEvent);
        void WakeSleepingThreads(TIMETYPE lTime);
        void NextPriority();
        
        /**
         * @brief Determines if the current priority queue is allocated execution time in the cycle.
         *
         * @details Implements a **Leaky Bucket / Deficit Round-Robin** scheduling algorithm:
         *          - Accumulates a time quota (`m_lPriority`) into the current priority's bucket (`m_lWaterJars[m_lPos]`).
         *          - Halves the quota (`m_lPriority >>= 1`) for exponentially lower weight at subsequent priority levels.
         *          - If the bucket reaches or exceeds the execution threshold (`0x100` / 256 ticks), 
         *            consumes the quota, approves execution, and advances to the next priority via `NextPriority()`.
         *
         * @return `true` if the current priority level is granted execution time; otherwise `false`.
         */
        bool DoesPriorityRun();
        
        void ScheduleEvents();
        static void SetEvent(ZEventBase* pEvent, ZScheduledEvent* pScheduledEvent);

        // members
        ZScheduledEventList m_pRunningThreads[16];
        ZScheduledEventList m_pSleepingThreads;
        uint16_t m_lWaterJars[16];
        int m_lPos;
        int m_lPriority;
    };
    RE_VERIFY_SIZE(ZScheduledUpdate, 0x6C);
}