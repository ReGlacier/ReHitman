#include <Glacier/Debug/sCall_u.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZSTL/STLport.h>
#include <Glacier/ZSTL/zstring.h>
#include <Windows.h>
#include <cstdarg>
#include <cstdio>


namespace Glacier
{
    namespace
    {
        stlp::set<zstring>* g_setOnce = nullptr;
        volatile long lLock_0 = 0;
        FILE* g_pOutputFile = nullptr;

        void AcquireSpinLock(volatile long* pLock)
        {
            while (InterlockedCompareExchange(pLock, 1, 0) != 0)
            {
                #if defined(_M_IX86) || defined(_M_X64)
                _mm_pause();
                #endif
            }
        }

        void ReleaseSpinLock(volatile long* pLock)
        {
            InterlockedExchange(pLock, 0);
        }

        void ZDebugWriteOutputFile(const uChannelEnums eChannel, ZDebug::uSeverityEnums eSeverity, const char* szString, int8_t flags)
        {
            if (!g_pOutputFile)
                return;

            if (flags & 2)
            {
                if (flags & 1)
                {
                    fprintf(g_pOutputFile, "%s\n", szString);
                }
                else
                {
                    fprintf(g_pOutputFile, "%s", szString);
                }
            }
            else
            {
                const char* psSeverity = "??";
                if (eSeverity < ZDebug::eSEV_MSG)
                {
                    switch (eSeverity)
                    {
                        case ZDebug::eSEV_ERROR:
                            psSeverity = "EE";
                            break;
                        case ZDebug::eSEV_WARN1:
                            psSeverity = "W1";
                            break;
                        case ZDebug::eSEV_WARN2:
                            psSeverity = "W2";
                            break;
                        case ZDebug::eSEV_WARN3:
                            psSeverity = "W3";
                            break;
                        default:
                            if (eSeverity)
                            {
                                psSeverity = "  ";
                            }
                            else
                            {
                                psSeverity = "--";
                            }
                    }
                }

                if (flags & 1)
                {
                    fprintf(g_pOutputFile, "[%s:%2d]  %s\n", psSeverity, eSeverity, szString);
                }
                else
                {
                    fprintf(g_pOutputFile, "[%s:%2d]  %s", psSeverity, eSeverity, szString);
                }
            }

            fflush(g_pOutputFile);
        }

        void ZDebugAssembleMessage(uChannelEnums eChannel, ZDebug::uSeverityEnums eSeverity, const char* szString, const char* szFile, const unsigned int iLine, const bool bIsConsoleLine, const bool bDisplayOnce)
        {
            if (!szString || !*szString)
                return;

            if (bDisplayOnce)
            {
                if (!g_setOnce)
                {
                    g_setOnce = new stlp::set<zstring>();
                }

                zstring strMessage(szString);

                if (g_setOnce->find(strMessage) != g_setOnce->end())
                {
                    return;
                }

                g_setOnce->insert(strMessage);
            }

            AcquireSpinLock(&lLock_0);

            if (g_pSysInterface && ZSysInterface::GetOption("TimeStampOnDebugOutput", nullptr))
            {
                float fSecs = g_pSysInterface->m_fRealPreTime.secs;
                float fFrameTime = fSecs * 0.0009765625f;
                int iFPS = static_cast<int>(fFrameTime);

                char szTimeStamp[128] { 0 };
                snprintf(szTimeStamp, sizeof(szTimeStamp), "FrameTime: %7.2f FPS:%3d ", fFrameTime, iFPS);

                OutputDebugStringA(szTimeStamp);
            }

            OutputDebugStringA(szString);
            OutputDebugStringA("\n");

            ReleaseSpinLock(&lLock_0);

            fflush(stdout);

            if (g_pOutputFile)
            {
                ZDebugWriteOutputFile(eChannel, eSeverity, szString, 1);
            }

            if (eSeverity == ZDebug::eSEV_ERROR)
            {
                ZHALT();
            }
        }
    }

    sCall_u::sCall_u(uint32_t iLine, const char* szFile, ZDebug::uSeverityEnums eSeverity, bool bDisplayOnce, bool bIsConsoleLine)
        : m_bIsConsoleLine(bIsConsoleLine)
        , m_bDisplayOnce(bDisplayOnce)
        , m_szFile(szFile)
        , m_iLine(iLine)
        , m_eSeverity(eSeverity)
    {
    }

    void sCall_u::operator->()
    {
        Dump();
    }

    void sCall_u::_uPrint(const char* fmt, ...)
    {
        char aMessageBuffer[2048] = { 0 };

        va_list args;
        va_start(args, fmt);
        vsnprintf(aMessageBuffer, sizeof(aMessageBuffer), fmt, args);
        va_end(args);

        ZDebugAssembleMessage(
            _chReserved,
            ZDebug::eSEV_PRINT,
            aMessageBuffer,
            m_szFile,
            m_iLine,
            m_bIsConsoleLine,
            m_bDisplayOnce
        );
    }

    void sCall_u::_uMsg(uChannelEnums eChannel, const char* fmt, ...)
    {
        if (ZDebug::g_eChannelSeverity[eChannel] < m_eSeverity)
        {
            return;
        }

        char aMessageBuffer[2048] = { 0 };

        va_list args;
        va_start(args, fmt);
        vsnprintf(aMessageBuffer, sizeof(aMessageBuffer), fmt, args);
        va_end(args);

        ZDebugAssembleMessage(
            eChannel,
            m_eSeverity,
            aMessageBuffer,
            m_szFile,
            m_iLine,
            m_bIsConsoleLine,
            m_bDisplayOnce
        );
    }

    void sCall_u::Dump()
    {
#       if defined(REHITMAN_USE_REAL_ZASSERT)
        if (!g_pSysInterface)
        {
            return;
        }

        char aMessageBuffer[4096] { 0 };

        const char* szSeverity = "UNKNOWN";
        switch (m_eSeverity)
        {
            case ZDebug::uSeverityEnums::eSEV_PRINT: szSeverity = "PRINT"; break;
            case ZDebug::uSeverityEnums::eSEV_ERROR: szSeverity = "ERROR"; break;
            case ZDebug::uSeverityEnums::eSEV_WARN1: szSeverity = "WARN1"; break;
            case ZDebug::uSeverityEnums::eSEV_WARN2: szSeverity = "WARN2"; break;
            case ZDebug::uSeverityEnums::eSEV_WARN3: szSeverity = "WARN3"; break;
            case ZDebug::uSeverityEnums::eSEV_MSG:   szSeverity = "MSG"; break;
            default: break;
        }
        const char* szFileName = m_szFile;
        if (m_szFile)
        {
            const char* szSlash = strrchr(m_szFile, '\\');
            if (!szSlash)
                szSlash = strrchr(m_szFile, '/');
            if (szSlash)
                szFileName = szSlash + 1;
        }

        snprintf(
            aMessageBuffer,
            sizeof(aMessageBuffer),
            "Assertion Failed!\n\n"
            "Severity: %s\n"
            "File: %s\n"
            "Line: %u\n"
            "Display Once: %s",
            szSeverity,
            szFileName ? szFileName : "Unknown",
            m_iLine,
            m_bDisplayOnce ? "Yes" : "No"
        );

        g_pSysInterface->DisplayAssert(aMessageBuffer, m_szFile, m_iLine);
#       endif
    }

    void ZDebugOpenOutputFile(const char* pszFileName)
    {
        if (g_pOutputFile)
        {
            fclose(g_pOutputFile);
        }

        g_pOutputFile = nullptr;
        if (pszFileName && pszFileName[0])
        {
            g_pOutputFile = fopen(pszFileName, "w");
        }
    }

    void ZDebugStampOutputFile(const char* pszBuildTag)
    {
        if (!pszBuildTag || !g_pOutputFile)
            return;

        __time64_t tm;
        _time64(&tm);
        fprintf(g_pOutputFile, "[-----] (@%lld) %s\n", tm, pszBuildTag);
        fflush(g_pOutputFile);
    }

    void DebugBrk()
    {
        DebugBreak();
    }
}
