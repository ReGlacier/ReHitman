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
    class ZDebugFrame;

    enum TIMERTYPE
    {
        TIMER_NONE = 0,
        TIMER_NORMAL = 1,
        TIMER_EVENTS = 2,
        TIMER_GEOMS = 3,
        TIMER_SINGLE = 4,
        TIMER_ONLINE = 5,
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
            int16_t m_lCount;
            int8_t m_lDepth;
            uint8_t m_Pad[1];
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

        struct ZOldDrawTimer
        {
            char m_Name[32];
            uint16_t m_lCount;
            int8_t m_lDepth;
            uint8_t m_Pad[1];
            uint32_t m_lColor;
            uint32_t m_lCycleTime;
        };

        // vtbl
        void Init() override;
        void Update() override;
        void BeginEdit() override;
        void DrawAlways() override;
        virtual void CalcDrawTimers(ZStackArray<64, ZDrawDebugTimer::ZOldDrawTimer>* pDrawTimers);

        // methods
        ZDrawDebugTimer();
        void End();
        void StartFastTimer(const char* pName);
        void EndFastTimer();
        void StartEventTimer(const char* pName);
        void EndEventTimer();
        void StartGeomTimer(const char* pName);
        void EndGeomTimer();
        void StartOnlineTimer(const char* pName);
        void EndOnlineTimer();
        void FrameUpdate();

        // members
        TIMERTYPE m_lTimerType;
        bool m_bMenuRunning;
        bool m_bRunning;
        float m_fScale;
        uint32_t m_lSelectedTimerItem;
        uint32_t m_lUpdateCount;
        ZFixedArray<ZDrawDebugTimer::ZTimer,128> m_Timers;
        ZStackArray<640, ZDrawDebugTimer::ZFastTimer> m_FastTimers;
        ZStackArray<512, ZDrawDebugTimer::ZTimer*> m_DrawTimers;
        ZTimer* m_HashLookup[256];
        TIMERTYPE m_eOldTimerType;
        ZPStrHash<uint32_t> m_PositionHash;
        uint32_t m_iHighPos;
        uint32_t m_iFramesSinceClear;
        TimerInfo_t m_aSingleTimers[32];
        TimerInfo_t* m_apSingleTimerStack[32];
        int m_lNumSingleTimers;
        int m_lCurSingleTimer;

    private:
        ZDrawDebugTimer::ZTimer* GetTimerLookup(ZDrawDebugTimer::ZTimer* pTimer, uint8_t lHash);
        void Cleanup();
        void StartTimer(ZStackArray<256, ZDrawDebugTimer::ZTimer*>* pTimersStack, const char* pName, uint64_t lTime);
        void EndTimer(ZStackArray<256, ZDrawDebugTimer::ZTimer*>* pTimersStack, uint64_t lTime);
        void DrawCycleBar(ZDebugFrame* pFrame, int lMenuSizeX, int lPlotLineY, uint32_t lCycles, uint32_t lColor);
        void DrawTimersList(ZDebugFrame* pFrame, ZStackArray<64, ZDrawDebugTimer::ZOldDrawTimer>* pDrawTimers, int lMenuSizeX, uint32_t lPlotLineY);
    };

    // Lazily constructs the global timer (Xbox g_DrawDebugTimer$initializer$) and binds g_pDrawDebugTimer.
    ZDrawDebugTimer& GetDrawDebugTimer();
}
