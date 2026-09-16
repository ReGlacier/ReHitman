#include <Glacier/Render/Debug/ZDrawDebugTimer.h>

#include <Glacier/Render/Debug/ZDebugFrame.h>
#include <Glacier/Render/Debug/ZDrawDebugText.h>
#include <Glacier/Render/Debug/Globals.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>

#include <cstdio>
#include <cstring>


namespace Glacier
{
    ZDrawDebugTimer::ZDrawDebugTimer()
        : m_PositionHash(0x10)
    {
        m_lTimerType = TIMER_NONE;
        m_bMenuRunning = false;
        m_bRunning = false;
        m_fScale = 3.0f;
        m_lSelectedTimerItem = 0xFFFFFFFFu;
        m_lUpdateCount = 0;
        m_eOldTimerType = TIMER_NONE;
        m_iHighPos = 0;
        m_iFramesSinceClear = 0;
        m_lNumSingleTimers = 0;
        m_lCurSingleTimer = 0;

        for (int i = 0; i < 256; ++i)
            m_HashLookup[i] = nullptr;

        for (int i = 0; i < 32; ++i)
            m_apSingleTimerStack[i] = nullptr;
    }

    void ZDrawDebugTimer::Init()
    {
        CountMenuItems();
        std::memset(m_HashLookup, 0, sizeof(m_HashLookup));
        m_Timers.Reset();
        m_lUpdateCount = 0;
    }

    void ZDrawDebugTimer::Update()
    {
        ZSimpleDebugMenu::Update();
    }

    void ZDrawDebugTimer::BeginEdit()
    {
        ZSimpleDebugMenu::BeginEdit();
    }

    void ZDrawDebugTimer::End()
    {
        // Do nothing
    }

    ZDrawDebugTimer::ZTimer* ZDrawDebugTimer::GetTimerLookup(ZDrawDebugTimer::ZTimer* pTimer, uint8_t lHash)
    {
        ZTimer* pEntry = m_HashLookup[lHash];
        if (!pEntry)
            return nullptr;

        while (pEntry->m_lDepth != pTimer->m_lDepth || std::strcmp(pEntry->m_Name, pTimer->m_Name) != 0)
        {
            pEntry = pEntry->m_pNextSameHash;
            if (!pEntry)
                return nullptr;
        }

        return pEntry;
    }

    void ZDrawDebugTimer::StartFastTimer(const char* pName)
    {
        if (!m_bRunning || m_lTimerType != TIMER_NORMAL)
            return;

        ZFastTimer sTimer;
        sTimer.m_lType = 0;
        std::strncpy(sTimer.m_Name, pName, sizeof(sTimer.m_Name) - 1);
        sTimer.m_Name[sizeof(sTimer.m_Name) - 1] = '\0';
        sTimer.m_lTime = static_cast<uint64_t>(g_pSysInterface->TimeStampCounter(__FILE__, __LINE__));

        m_FastTimers.Add(&sTimer);
    }

    void ZDrawDebugTimer::EndFastTimer()
    {
        if (!m_bRunning || m_lTimerType != TIMER_NORMAL)
            return;

        ZFastTimer sTimer;
        sTimer.m_lType = 1;
        sTimer.m_lTime = static_cast<uint64_t>(g_pSysInterface->TimeStampCounter(__FILE__, __LINE__));

        m_FastTimers.Add(&sTimer);
    }

    void ZDrawDebugTimer::StartEventTimer(const char* pName)
    {
        if (!m_bRunning || m_lTimerType != TIMER_EVENTS)
            return;

        ZFastTimer sTimer;
        sTimer.m_lType = 0;
        std::strncpy(sTimer.m_Name, pName, sizeof(sTimer.m_Name) - 1);
        sTimer.m_Name[sizeof(sTimer.m_Name) - 1] = '\0';
        sTimer.m_lTime = static_cast<uint64_t>(g_pSysInterface->TimeStampCounter(__FILE__, __LINE__));

        m_FastTimers.Add(&sTimer);
    }

    void ZDrawDebugTimer::EndEventTimer()
    {
        if (!m_bRunning || m_lTimerType != TIMER_EVENTS)
            return;

        ZFastTimer sTimer;
        sTimer.m_lType = 1;
        sTimer.m_lTime = static_cast<uint64_t>(g_pSysInterface->TimeStampCounter(__FILE__, __LINE__));

        m_FastTimers.Add(&sTimer);
    }

    void ZDrawDebugTimer::StartGeomTimer(const char* pName)
    {
        if (!m_bRunning || m_lTimerType != TIMER_GEOMS)
            return;

        ZFastTimer sTimer;
        sTimer.m_lType = 0;
        std::strncpy(sTimer.m_Name, pName, sizeof(sTimer.m_Name) - 1);
        sTimer.m_Name[sizeof(sTimer.m_Name) - 1] = '\0';
        sTimer.m_lTime = static_cast<uint64_t>(g_pSysInterface->TimeStampCounter(__FILE__, __LINE__));

        m_FastTimers.Add(&sTimer);
    }

    void ZDrawDebugTimer::EndGeomTimer()
    {
        if (!m_bRunning || m_lTimerType != TIMER_GEOMS)
            return;

        ZFastTimer sTimer;
        sTimer.m_lType = 1;
        sTimer.m_lTime = static_cast<uint64_t>(g_pSysInterface->TimeStampCounter(__FILE__, __LINE__));

        m_FastTimers.Add(&sTimer);
    }

    void ZDrawDebugTimer::StartOnlineTimer(const char* pName)
    {
        if (!m_bRunning || m_lTimerType != TIMER_ONLINE)
            return;

        ZFastTimer sTimer;
        sTimer.m_lType = 0;
        std::strncpy(sTimer.m_Name, pName, sizeof(sTimer.m_Name) - 1);
        sTimer.m_Name[sizeof(sTimer.m_Name) - 1] = '\0';
        sTimer.m_lTime = static_cast<uint64_t>(g_pSysInterface->TimeStampCounter(__FILE__, __LINE__));

        m_FastTimers.Add(&sTimer);
    }

    void ZDrawDebugTimer::EndOnlineTimer()
    {
        if (!m_bRunning || m_lTimerType != TIMER_ONLINE)
            return;

        ZFastTimer sTimer;
        sTimer.m_lType = 1;
        sTimer.m_lTime = static_cast<uint64_t>(g_pSysInterface->TimeStampCounter(__FILE__, __LINE__));

        m_FastTimers.Add(&sTimer);
    }

    void ZDrawDebugTimer::StartTimer(ZStackArray<256, ZDrawDebugTimer::ZTimer*>* pTimersStack, const char* pName, uint64_t lTime)
    {
        ZTimer sLookup;
        std::strncpy(sLookup.m_Name, pName, sizeof(sLookup.m_Name) - 1);
        sLookup.m_Name[sizeof(sLookup.m_Name) - 1] = '\0';
        sLookup.m_lDepth = static_cast<int8_t>(pTimersStack->m_lNrEntries);
        sLookup.m_lColor = 0x00404040u;

        const uint8_t lBucket = static_cast<uint8_t>(*pName);
        ZTimer* pTimer = GetTimerLookup(&sLookup, lBucket);

        if (!pTimer)
        {
            pTimer = m_Timers.Add();
            ZASSERT(pTimer);

            sLookup.m_lCount = 0;
            sLookup.m_lLastUpdateFrame = 0;
            sLookup.m_pNextSameHash = m_HashLookup[lBucket];
            sLookup.m_lStart = 0;
            sLookup.m_lDiff = 0;
            *pTimer = sLookup;

            m_HashLookup[lBucket] = pTimer;
            pTimer->m_lLastUpdateFrame = 0xFFFFFFFFu;
        }

        if (pTimer->m_lLastUpdateFrame == m_lUpdateCount)
        {
            ++pTimer->m_lCount;
        }
        else
        {
            pTimer->m_lLastUpdateFrame = m_lUpdateCount;
            m_DrawTimers.m_Array[m_DrawTimers.m_lNrEntries] = pTimer;
            ++m_DrawTimers.m_lNrEntries;
            ZASSERT(m_DrawTimers.m_lNrEntries <= 512);
        }

        pTimer->m_lStart = lTime;

        pTimersStack->m_Array[pTimersStack->m_lNrEntries] = pTimer;
        ++pTimersStack->m_lNrEntries;
        ZASSERT(pTimersStack->m_lNrEntries <= 256);
    }

    void ZDrawDebugTimer::EndTimer(ZStackArray<256, ZDrawDebugTimer::ZTimer*>* pTimersStack, uint64_t lTime)
    {
        if (!pTimersStack->m_lNrEntries)
            return;

        ZTimer* pTimer = pTimersStack->m_Array[pTimersStack->m_lNrEntries - 1];
        pTimersStack->Remove(pTimersStack->m_lNrEntries - 1);

        pTimer->m_lDiff += lTime - pTimer->m_lStart;
    }

    void ZDrawDebugTimer::Cleanup()
    {
        uint32_t i = 0;
        while (i != m_DrawTimers.m_lNrEntries)
        {
            if (m_DrawTimers.m_Array[i]->m_lLastUpdateFrame != m_lUpdateCount)
            {
                m_DrawTimers.Remove(i);
                --i;
            }
            ++i;
        }

        for (int b = 0; b < 256; ++b)
        {
            ZTimer** ppLink = &m_HashLookup[b];
            ZTimer* pEntry = *ppLink;

            while (pEntry)
            {
                if (pEntry->m_lLastUpdateFrame == m_lUpdateCount)
                {
                    ppLink = &pEntry->m_pNextSameHash;
                    pEntry = pEntry->m_pNextSameHash;
                }
                else
                {
                    *ppLink = pEntry->m_pNextSameHash;
                    m_Timers.Remove(pEntry);
                    pEntry = *ppLink;
                }
            }
        }
    }

    void ZDrawDebugTimer::FrameUpdate()
    {
        m_bRunning = true;

        uint32_t lDisplay = g_lTimerDisplayEnabled;
        if (!g_lTimerDisplayEnabled)
        {
            m_bRunning = false;
            lDisplay = g_lTimerDisplayEnabled;
        }

        if (!m_bRunning)
        {
            if (m_Timers.Count())
            {
                std::memset(m_HashLookup, 0, sizeof(m_HashLookup));
                m_FastTimers.m_lNrEntries = 0;
                m_Timers.Reset();
                m_lUpdateCount = 0;
                m_lTimerType = TIMER_NONE;
            }
            return;
        }

        switch (lDisplay)
        {
            case 1: m_lTimerType = TIMER_NORMAL; break;
            case 2: m_lTimerType = TIMER_GEOMS; break;
            case 3: m_lTimerType = TIMER_EVENTS; break;
            case 4: m_lTimerType = TIMER_SINGLE; break;
            case 5: m_lTimerType = TIMER_ONLINE; break;
            default: break;
        }

        ZStackArray<256, ZTimer*> aSingleTimersStack;
        aSingleTimersStack.Clear();

        m_DrawTimers.m_lNrEntries = 0;
        ++m_lUpdateCount;

        const uint32_t lNumFastTimers = m_FastTimers.m_lNrEntries;
        for (uint32_t i = 0; i < lNumFastTimers && i < m_FastTimers.m_lNrEntries; ++i)
        {
            const ZFastTimer* pFastTimer = &m_FastTimers.m_Array[i];

            if (pFastTimer->m_lType == 0)
                StartTimer(&aSingleTimersStack, pFastTimer->m_Name, pFastTimer->m_lTime);
            else if (pFastTimer->m_lType == 1)
                EndTimer(&aSingleTimersStack, pFastTimer->m_lTime);
            else if (pFastTimer->m_lType >= 3)
                continue;
            else
                break;
        }

        Cleanup();
        m_FastTimers.m_lNrEntries = 0;

        for (int i = 0; i < m_lNumSingleTimers; ++i)
        {
            STimeInfo& timer = m_aSingleTimers[i];

            if (timer.lTotal > timer.lHigh || ++timer.lTicksSinceHigh > 100)
            {
                const int lCalls = timer.lCallCount;
                timer.lTicksSinceHigh = 0;
                timer.lHigh = timer.lTotal;
                timer.lHighCount = lCalls;
            }

            timer.lCallCount = 0;
            timer.lTotal = 0;
        }
    }

    void ZDrawDebugTimer::DrawCycleBar(ZDebugFrame* pFrame, int lMenuSizeX, int lPlotLineY, uint32_t lCycles, uint32_t lColor)
    {
        const float fFrameScale = 60.0f / m_fScale;
        const float fFrameTime = static_cast<float>(g_pSysInterface->m_fActualTime.secs) / fFrameScale;
        const int lBarWidth = 9 * (lMenuSizeX - 7) - 128;

        int lBar = static_cast<int>(static_cast<float>(lCycles) / fFrameTime * static_cast<float>(lBarWidth));
        if (lBar > lBarWidth)
            lBar = lBarWidth;

        pFrame->DrawRect(128, 15 * lPlotLineY + 1, lBar, 14, lColor | 0xFF000000u);
    }

    void ZDrawDebugTimer::CalcDrawTimers(ZStackArray<64, ZDrawDebugTimer::ZOldDrawTimer>* pDrawTimers)
    {
        pDrawTimers->m_lNrEntries = 0;

        uint32_t aPosStack[32];
        int lStackCount = 0;

        for (uint32_t i = 0; i < m_DrawTimers.m_lNrEntries; ++i)
        {
            ZTimer* pTimer = m_DrawTimers.m_Array[i];

            while (lStackCount > 0 && lStackCount > pTimer->m_lDepth)
                --lStackCount;

            ZOldDrawTimer sOldTimer;
            std::strncpy(sOldTimer.m_Name, pTimer->m_Name, sizeof(sOldTimer.m_Name) - 1);
            sOldTimer.m_Name[sizeof(sOldTimer.m_Name) - 1] = '\0';
            sOldTimer.m_lCount = static_cast<uint16_t>(pTimer->m_lCount);
            sOldTimer.m_lDepth = pTimer->m_lDepth;
            sOldTimer.m_Pad[0] = 0;
            sOldTimer.m_lColor = pTimer->m_lColor;
            sOldTimer.m_lCycleTime = static_cast<uint32_t>(pTimer->m_lDiff);

            pDrawTimers->Add(&sOldTimer);
            aPosStack[lStackCount] = pDrawTimers->m_lNrEntries - 1;
            ++lStackCount;

            for (int k = 0; k < lStackCount - 1; ++k)
                pDrawTimers->m_Array[aPosStack[k]].m_lCycleTime -= static_cast<uint32_t>(pTimer->m_lDiff);

            pTimer->m_lDiff = 0;
            pTimer->m_lCount = 1;
        }
    }

    void ZDrawDebugTimer::DrawTimersList(ZDebugFrame* pFrame, ZStackArray<64, ZDrawDebugTimer::ZOldDrawTimer>* pDrawTimers, int lMenuSizeX, uint32_t lPlotLineY)
    {
        ZASSERT(lMenuSizeX < 256);

        const float fFrameScale = static_cast<float>(g_pSysInterface->m_fActualTime.secs) / static_cast<float>(g_pSysInterface->m_lFrameCount);

        if (m_lTimerType == TIMER_NORMAL)
        {
            pFrame->DrawRect(128, 0, 9 * lMenuSizeX - 191, 15 * static_cast<int>(pDrawTimers->m_lNrEntries), 0x80000000u);

            const float fInvFrameScale = 1.0f / fFrameScale;
            for (uint32_t i = 0; i < pDrawTimers->m_lNrEntries; ++i)
            {
                ZOldDrawTimer* pTimer = pDrawTimers->Get(i);

                char szText[256];
                std::strncpy(szText, pTimer->m_Name, lMenuSizeX);
                szText[lMenuSizeX] = '\0';

                DrawCycleBar(pFrame, lMenuSizeX, lPlotLineY, pTimer->m_lCycleTime, pTimer->m_lColor);

                if (m_lSelectedTimerItem == i)
                    pFrame->PlotFixedInvers(0, lPlotLineY, szText, "%d %.3f", static_cast<int>(pTimer->m_lCycleTime), static_cast<double>(pTimer->m_lCycleTime) * fInvFrameScale);
                else
                    pFrame->PlotFixed(0, lPlotLineY, szText, "%d %.3f", static_cast<int>(pTimer->m_lCycleTime), static_cast<double>(pTimer->m_lCycleTime) * fInvFrameScale);

                ++lPlotLineY;
            }

            const int lGridWidth = 9 * lMenuSizeX - 191;
            const int lNumLines = static_cast<int>(m_fScale + 1.01f);
            for (int i = 0; i < lNumLines; ++i)
            {
                const float fX = static_cast<float>(i) / m_fScale * static_cast<float>(lGridWidth);
                if (static_cast<int>(fX) <= lGridWidth)
                {
                    pFrame->DrawLine(static_cast<float>(pFrame->m_lSizeX), 0.0f,
                        static_cast<float>(pFrame->m_lSizeX), static_cast<float>(pFrame->m_lSizeY), 0xFFFFFFFFu);
                }
            }
        }
        else if (m_lTimerType == TIMER_SINGLE)
        {
            for (int i = 0; i < m_lNumSingleTimers; ++i)
            {
                STimeInfo& timer = m_aSingleTimers[i];

                char szText[64];
                std::memset(szText, ' ', 40);
                std::strncpy(szText, timer.pzName ? timer.pzName : "", 40);
                szText[40] = '\0';

                pFrame->PlotFixed(0, lPlotLineY, szText, "%4d %7d [%4d %7d]",
                    timer.lCallCount, timer.lTotal, timer.lHigh, timer.lHighCount);

                ++lPlotLineY;
            }
        }
        else
        {
            uint32_t lTotalCount = 0;
            uint32_t lTotalCycle = 0;

            for (uint32_t i = 0; i < pDrawTimers->m_lNrEntries; ++i)
            {
                ZOldDrawTimer* pTimer = pDrawTimers->Get(i);
                lTotalCount += pTimer->m_lCount;
                lTotalCycle += pTimer->m_lCycleTime;
            }

            const float fInvFrameScale = 1.0f / fFrameScale;

            char szTitle[256];
            std::strncpy(szTitle, m_lTimerType == TIMER_GEOMS ? "Geom FrameUpdate" : "Event FrameUpdate", lMenuSizeX);
            szTitle[lMenuSizeX] = '\0';

            if (m_eOldTimerType != m_lTimerType || m_iFramesSinceClear > 300)
            {
                m_PositionHash.Clear();
                m_iHighPos = 0;
                m_eOldTimerType = m_lTimerType;
                m_iFramesSinceClear = 0;
            }

            ++m_iFramesSinceClear;

            for (uint32_t i = 0; i < pDrawTimers->m_lNrEntries; ++i)
            {
                ZOldDrawTimer* pTimer = pDrawTimers->Get(i);

                if (!m_PositionHash.Contains(pTimer->m_Name))
                    m_PositionHash.Put(pTimer->m_Name, m_iHighPos++);
            }

            pFrame->PlotFixed(0, lPlotLineY, szTitle, "%d %.3f",
                static_cast<int>(lTotalCount), static_cast<double>(lTotalCycle) * fInvFrameScale);

            const uint32_t lNumCols = (m_iHighPos + 3) >> 1;
            const int lColWidth = lMenuSizeX - (lMenuSizeX >> 1) - 1;

            for (uint32_t i = 0; i < pDrawTimers->m_lNrEntries; ++i)
            {
                ZOldDrawTimer* pTimer = pDrawTimers->Get(i);

                const uint32_t* pPos = m_PositionHash.Get(pTimer->m_Name);
                if (!pPos)
                    continue;

                char szText[256];
                std::strncpy(szText, pTimer->m_Name, pTimer->m_lDepth < 200 ? pTimer->m_lDepth : 200);
                szText[pTimer->m_lDepth] = '\0';

                if (lNumCols > 1)
                {
                    pFrame->PlotFixed(
                        static_cast<uint32_t>((*pPos / (lNumCols - 1)) * lColWidth),
                        (*pPos % (lNumCols - 1)) + 1 + lPlotLineY,
                        szText, "%d %.3f",
                        static_cast<int>(pTimer->m_lCycleTime),
                        static_cast<double>(pTimer->m_lCycleTime) * fInvFrameScale);
                }
            }
        }
    }

    void ZDrawDebugTimer::DrawAlways()
    {
        if (!m_bRunning)
            return;

        ZStackArray<64, ZOldDrawTimer> aOldDrawTimers;
        aOldDrawTimers.m_lNrEntries = 0;
        CalcDrawTimers(&aOldDrawTimers);

        ZDebugFrame* pFrame = g_pDrawDebugText->AddFrame();
        ZDebugFrame frame { pFrame };

        const int lMenuX = frame.m_pParent->m_lSizeX / 9;
        int lSizeX = lMenuX - 22;
        if (!g_bInfoDisplayEnabled)
            lSizeX = lMenuX - 1;

        const int lDrawX = lSizeX - 18;

        int lNumLines;
        if (m_lTimerType == TIMER_SINGLE)
            lNumLines = m_lNumSingleTimers;
        else if (m_lTimerType == TIMER_NORMAL)
            lNumLines = static_cast<int>(aOldDrawTimers.m_lNrEntries);
        else
            lNumLines = static_cast<int>(((m_PositionHash.Count() + 1) >> 1) + 1);

        frame.SetPosSizeText(9, 4, lSizeX - 18, lNumLines);
        frame.Fill(0x80202080u);
        frame.DrawFocus();
        DrawTimersList(&frame, &aOldDrawTimers, lDrawX, 0);
    }

    // Xbox_KL1 sub_82D0BCF0 / KL2 dynamic_initializer_for__g_DrawDebugTimer__: a namespace-scope
    // ZDrawDebugTimer is constructed during static initialization and registered as the active
    // timer. ZUniMemory falls back to the C heap while the game's ISysMem is not up yet, so the
    // m_PositionHash allocation performed here is safe.
    ZDrawDebugTimer g_DrawDebugTimer;

    namespace
    {
        struct ZDrawDebugTimerRegistrar
        {
            ZDrawDebugTimerRegistrar()
            {
                g_pDrawDebugTimer = &g_DrawDebugTimer;
            }
        };

        ZDrawDebugTimerRegistrar g_DrawDebugTimerRegistrar;
    }

    ZDrawDebugTimer& GetDrawDebugTimer()
    {
        return g_DrawDebugTimer;
    }
}
