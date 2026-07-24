#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/TIMETYPE.h>
#include <Glacier/ZSTL/ZPoolAllocRefTab.h> // ZPoolAllocator
#include <cstdint>

namespace Glacier 
{
    class ZEventBase;

    class ZScheduledEvent 
    {
    public:
        // types
        enum EScheduledEventState 
        {
            STATE_RUNNING = 0x40000000u
        };

        // constants
        static constexpr uint32_t MAX_PRIORITIES_NR = 16;

        // static
        static ZREF s_rEvent;
        static uint8_t s_PoolAllocatorBuffer[0x4000];
        static ZPoolAllocator s_PoolAllocator;

        // vtbl
        virtual void Run();
        virtual ZScheduledEvent* GetNextThread();
        virtual ZScheduledEvent* Fork();

        // methods
        ZScheduledEvent(ZEventBase* pEvent);
        ZScheduledEvent* Fork(ZScheduledEvent* pEvent);
        const uint32_t SetPriority(uint32_t lPriority);
        const uint32_t GetPriority() const;
        const float GetSleepTimeLeft() const;
        bool HasThreads() const;

        // static methods
        static void EnterFunction();

        // operators
        static void* operator new(std::size_t size);
        static void operator delete(void* ptr);

        // members
        ZREF m_rEvent;
        ZScheduledEvent* m_pNxt{nullptr};
        ZScheduledEvent* m_pThread{nullptr};
        union
        {
            char* m_pStoredStack{nullptr};
            void* m_pUserData;
        };
        uint32_t m_bChanged : 1;
        uint32_t m_bRunning : 1;
        uint32_t m_lStoredStackLength : 26;
        uint32_t m_lPriority : 4;
        TIMETYPE m_tNextRun{};
    };
    RE_VERIFY_SIZE(ZScheduledEvent, 0x1C);
    RE_VERIFY_OFFSET(ZScheduledEvent, m_tNextRun, 0x18);
}