#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZHash.h>
#include <Glacier/ZSTL/TIMETYPE.h>
#include <Glacier/ZSTL/ZStackArray.h>
#include <Glacier/ZSTL/ZFixedArray.h>
#include <Glacier/Render/Debug/ZSimpleDebugMenu.h>
#include <cstdint>


namespace Glacier
{
    enum TIMERTYPE
    {
        NONE = 0,
        NORMAL = 1,
        EVENTS = 2,
        GEOMS = 3,
        SINGLE = 4,
    };

    struct STimeInfo
    {
        const char* pzName;
        int lCallCount;
        int lTotal;
        int lHigh;
        int lHighCount;
        int lTicksSinceHigh;
        int64_t lStartTime;
    };

    using TimerInfo_t = STimeInfo;

    class ZDrawDebugTimer : public ZSimpleDebugMenu
    {
    public:
        // types
        class ZTimer
        {
        public:
            // members
            char m_Name[32];
            uint32_t m_lColor;
            int8_t m_lDepth;
            int8_t m_lCount;
            uint8_t m_Pad[2];
            uint32_t m_lLastUpdateFrame;
            ZTimer* m_pNextSameHash;
            uint64_t m_lStart;
            uint64_t m_lDiff;
        };

        class ZFastTimer
        {
        public:
            uint32_t m_lType;
            char m_Name[32];
            uint64_t m_lTime;
        };

        // methods
        ZDrawDebugTimer();

        // members
        TIMERTYPE m_lTimerType;
        bool m_bMenuRunning;
        bool m_bRunning;
        float m_fScale;
        uint32_t m_lSelectedTimerItem;
        uint32_t m_lUpdateCount;
        ZFixedArray<ZDrawDebugTimer::ZTimer,128> m_Timers;
        ZStackArray<640,ZDrawDebugTimer::ZFastTimer> m_FastTimers;
        ZStackArray<512,ZDrawDebugTimer::ZTimer*> m_DrawTimers;
        ZTimer* m_HashLookup[256];
        TIMERTYPE m_eOldTimerType;
        ZStrHash<uint32_t> m_PositionHash;
        uint32_t m_iHighPos;
        uint32_t m_iFramesSinceClear;
        TimerInfo_t m_aSingleTimers[32];
        TimerInfo_t* m_apSingleTimerStack[32];
        int m_lNumSingleTimers;
        int m_lCurSingleTimer;
    };
}
