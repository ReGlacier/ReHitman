#include <Glacier/Render/Debug/ZDrawDebugTextLayer.h>
#include <Glacier/Render/Debug/ZDrawDebugRender.h>
#include <Glacier/Render/Debug/Globals.h>

#include <Glacier/Debug/console.h>

#include <Glacier/System/ZSysInterface.h>

#include <Glacier/ZUniMemory.h>

#include <cstdio>
#include <cstring>
#include <cmath>


namespace Glacier
{
    namespace
    {
        const char* g_pszBuildId = "ReHitman";

        struct SHelpEntry
        {
            uint32_t lColor;
            uint32_t lStartX;
            uint32_t lStartY;
            const char* pText;
        };

        const SHelpEntry HelpDataList[] =
        {
            { 0xFFFFFFFFu, 1,  1,  "Enter       - execute command" },
            { 0xFFFFFFFFu, 1,  2,  "Esc         - hide console" },
            { 0xFFFFFFFFu, 1,  3,  "Up/Down     - scroll history" },
            { 0xFFFFFFFFu, 1,  4,  "Tab         - complete / list commands" },
            { 0xFFFFFFFFu, 1,  5,  "Home/End    - first/last history entry" },
            { 0xFFFFFFFFu, 1,  6,  "Backspace   - delete previous character" },
            { 0xFFFFFFFFu, 1,  7,  "Left/Right  - move cursor" },
            { 0xFFFFFFFFu, 1,  8,  "h           - toggle this help panel" },
        };
    }

    ZDrawDebugTextLayer::ZDrawDebugTextLayer()
    {
        m_pszTextBuffer = nullptr;
        m_lSizeX = 1;
        m_lSizeY = 1;
        m_lMarginX = 0;
        m_lMarginY = 0;
        m_lTextColor = 0xFFFFFFFFu;
        m_bDisplayHelp = false;
    }

    ZDrawDebugTextLayer::~ZDrawDebugTextLayer()
    {
        if (m_pszTextBuffer)
            ZUniMemory::DeleteArray(m_pszTextBuffer, m_lSizeX * m_lSizeY);
    }

    void ZDrawDebugTextLayer::FrameUpdate(float)
    {
        // Do nothing
    }

    ZDrawDebugTextLayer& ZDrawDebugTextLayer::Instance()
    {
        static ZDrawDebugTextLayer layer;
        return layer;
    }

    void ZDrawDebugTextLayer::SetSize(uint32_t x, uint32_t y)
    {
        m_lMarginX = 10;
        m_lMarginY = 2;
        m_lSizeX = x - 20;
        m_lSizeY = y - 4;

        if (m_pszTextBuffer)
            ZUniMemory::DeleteArray(m_pszTextBuffer, m_lSizeX * m_lSizeY);

        m_pszTextBuffer = ZUniMemory::NewArray<char>(m_lSizeX * m_lSizeY);
        std::memset(m_pszTextBuffer, 0, m_lSizeX * m_lSizeY);
    }

    void ZDrawDebugTextLayer::Write(uint32_t lX, uint32_t lY, const char* pszText)
    {
        if (lX >= m_lSizeX || lY >= m_lSizeY)
            return;

        char* pDest = &m_pszTextBuffer[m_lSizeX * lY + lX];
        char* pEnd = pDest + m_lSizeX - lX;

        while (*pszText && pDest < pEnd)
            *pDest++ = *pszText++;

        if (pDest < pEnd)
            *pDest = '\0';
    }

    void ZDrawDebugTextLayer::DrawHelp(ZDrawDebugRender* pRender)
    {
        const float fFontSizeX = pRender->m_fFontSize.x;
        const float fFontSizeY = pRender->m_fFontSize.y;

        float mText[9];
        mreset(mText);

        float sScale[3] = { fFontSizeX, fFontSizeY, 0.0f };

        for (size_t i = 0; i < sizeof(HelpDataList) / sizeof(HelpDataList[0]); ++i)
        {
            const SHelpEntry& entry = HelpDataList[i];
            float vPos[3] = { (float)pRender->m_Viewport.x + entry.lStartX * fFontSizeX,
                              (float)pRender->m_Viewport.y + entry.lStartY * fFontSizeY,
                              0.0f };
            pRender->xDrawText(mText, vPos, sScale, entry.pText,
                static_cast<uint32_t>(std::strlen(entry.pText)), entry.lColor, 0);
        }
    }

    void ZDrawDebugTextLayer::DrawConsole(ZDrawDebugRender* pRender)
    {
        CConsole* pConsole = g_pConsole;
        if (pConsole == nullptr)
            return;

        const float fOffset = pConsole->GetOffset();
        if (fOffset == 0.0f)
            return;

        const float fFontSizeX = pRender->m_fFontSize.x;
        const float fFontSizeY = pRender->m_fFontSize.y;
        const float fViewportX = (float)pRender->m_Viewport.x;
        const float fViewportY = (float)pRender->m_Viewport.y;
        const float fViewportW = (float)pRender->m_Viewport.w;
        const float fViewportH = (float)pRender->m_Viewport.h;

        const float fLeft = fFontSizeX * 0.15f;
        const float fRight = fViewportW - fLeft;
        const float fPanelTop = fViewportH - fOffset * (fViewportH * 0.5f);
        const float fPanelBottom = fViewportH;

        // Background panel.
        const uint32_t lAlpha = static_cast<uint32_t>(fOffset * 204.0f);
        const uint32_t lClampedAlpha = lAlpha > 255 ? 255u : lAlpha;
        pRender->xSetTexture(0u);
        pRender->xSetDrawMode(0x20001);
        pRender->xColor((lClampedAlpha << 24) | 0x202030);
        pRender->xBegin(ZDrawDebugRender::PT_TRIANGLES);

        float vBL[3] = { fViewportX + fLeft, fPanelBottom, 0.0f };
        float vBR[3] = { fViewportX + fRight, fPanelBottom, 0.0f };
        float vTR[3] = { fViewportX + fRight, fPanelTop, 0.0f };
        float vTL[3] = { fViewportX + fLeft, fPanelTop, 0.0f };
        pRender->xVertex2v(vBL);
        pRender->xVertex2v(vBR);
        pRender->xVertex2v(vTR);
        pRender->xVertex2v(vBL);
        pRender->xVertex2v(vTR);
        pRender->xVertex2v(vTL);
        pRender->xEnd();

        // Console text.
        float mText[9];
        mreset(mText);
        float sScale[3] = { fFontSizeX, fFontSizeY, 0.0f };

        uint32_t lMaxChars = static_cast<uint32_t>((fViewportW / fFontSizeX) * 0.85f);
        if (lMaxChars < 1)
            lMaxChars = 1;

        int lMaxLines = static_cast<int>((fPanelBottom - fPanelTop) / fFontSizeY);
        if (lMaxLines > 256)
            lMaxLines = 256;

        float fY = fPanelBottom - fFontSizeY;
        for (int i = 0; i < lMaxLines; ++i)
        {
            fY -= fFontSizeY;
            if (fY < fPanelTop)
                break;

            const char* pLine = (i == 0) ? pConsole->GetLine(0) : pConsole->GetLine(-i);
            if (pLine == nullptr)
                break;

            uint32_t lLen = static_cast<uint32_t>(std::strlen(pLine));
            if (lLen > lMaxChars)
                lLen = lMaxChars;

            float vPos[3] = { fViewportX + fFontSizeX, fY, 0.0f };
            pRender->xDrawText(mText, vPos, sScale, pLine, lLen, 0xFFFFFFFFu, 0);
        }
    }

    void ZDrawDebugTextLayer::DrawFullScreen(ZDrawDebugRender* pRender)
    {
        if (!g_lShowDebugText.m_iValue || !g_lShowDebug.m_iValue)
            return;

        if (m_bDisplayHelp)
        {
            DrawHelp(pRender);
        }
        else
        {
            pRender->xSetDrawMode(0x60001);
            DrawConsole(pRender);
        }
    }
}
