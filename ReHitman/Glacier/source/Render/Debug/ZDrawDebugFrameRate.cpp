#include <Glacier/Render/Debug/ZDrawDebugFrameRate.h>

#include <Glacier/Render/Debug/ZDrawDebugRender.h>
#include <Glacier/Render/Debug/Globals.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZSTL/ZMath.h>

#include <cstdio>
#include <cstring>


namespace Glacier
{
    ZDrawDebugFrameRate::ZDrawDebugFrameRate()
    {
        m_lFrameCountPos = 0;
        std::memset(m_lFrameCount, 0, sizeof(m_lFrameCount));
    }

    void ZDrawDebugFrameRate::DrawFullScreen(ZDrawDebugRender* pRender)
    {
        if (!g_lShowFrameRate.m_iValue || !g_lShowDebug.m_iValue)
            return;

        float fFrameTime = g_pSysInterface->DeltaFrameTime;
        if (fFrameTime <= 0.0f)
            fFrameTime = 1.0f;

        int lFPS = static_cast<int>(1.0f / fFrameTime + 0.5f);
        if (lFPS >= 65535)
            lFPS = 65535;

        m_lFrameCount[m_lFrameCountPos] = lFPS;
        if (++m_lFrameCountPos == 128)
            m_lFrameCountPos = 0;

        const float fRight = static_cast<float>(pRender->m_Viewport.w - 1);
        const float fTop = static_cast<float>(pRender->m_Viewport.h - 1);

        const float x0 = fRight;
        const float x1 = fRight * 0.85f;
        const float y0 = fTop * 0.0f;
        const float y1 = fTop * 0.15f;

        ZDrawDebugRender* r = pRender;

        r->xSetDrawMode(0x60001u);
        r->xColor(0x1FAFAFFFu);
        r->xBegin(ZDrawDebugRender::PT_TRIANGLES);
        r->xVertex2(x1, y0);
        r->xVertex2(x0, y0);
        r->xVertex2(x0, y1);
        r->xVertex2(x1, y0);
        r->xVertex2(x0, y1);
        r->xVertex2(x1, y1);
        r->xEnd();

        r->xBegin(ZDrawDebugRender::PT_LINES);
        r->xColor(0x1FFFFFFFu);
        r->xVertex2(x1, y0);
        r->xVertex2(x1, y1);
        r->xVertex2(x1, y1);
        r->xVertex2(x0, y1);
        r->xVertex2(x0 - 1.0f, y1 - 1.0f);
        r->xVertex2(x0 - 1.0f, y0 + 1.0f);
        r->xVertex2(x0 - 1.0f, y0 + 1.0f);
        r->xVertex2(x1 + 1.0f, y0 + 1.0f);

        r->xColor(0x1F000000u);
        r->xVertex2(x1 + 1.0f, y0 + 1.0f);
        r->xVertex2(x1 + 1.0f, y1 - 1.0f);
        r->xVertex2(x1 + 1.0f, y1 - 1.0f);
        r->xVertex2(x0 - 1.0f, y1 - 1.0f);
        r->xVertex2(x0, y1);
        r->xVertex2(x0, y0);
        r->xVertex2(x0, y0);
        r->xVertex2(x1, y0);
        r->xEnd();

        const float fHeight = y1 - y0;

        r->xBegin(ZDrawDebugRender::PT_LINES);
        r->xColor(0xFF3F3F4Fu);
        for (int i = 0; i < 4; ++i)
        {
            const float fY = static_cast<float>(i) * 0.25f * fHeight + y0;
            r->xVertex2(x1, fY);
            r->xVertex2(x0, fY);
        }
        r->xEnd();

        r->xBegin(ZDrawDebugRender::PT_LINES);
        r->xColor(0xFF7F7F9Fu);
        r->xVertex2(x1, fHeight * 0.6f + y0);
        r->xVertex2(x0, fHeight * 0.6f + y0);
        r->xEnd();

        const float fGraphWidth = x0 - x1;
        const float fStep = fGraphWidth * 0.0078125f;

        uint32_t lIndex = m_lFrameCountPos;
        uint32_t lNextIndex = m_lFrameCountPos + 1;
        float fX = x1;

        r->xBegin(ZDrawDebugRender::PT_LINES);
        r->xColor(0xFF7F3F3Fu);
        for (int i = 127; i; --i)
        {
            const float fCur = static_cast<float>(m_lFrameCount[lIndex & 0x7F]) * fHeight * 0.01f + y0;
            const float fNext = static_cast<float>(m_lFrameCount[lNextIndex & 0x7F]) * fHeight * 0.01f + y0;

            r->xVertex2(fX, fCur);
            fX += fStep;
            r->xVertex2(fX, fNext);

            ++lIndex;
            ++lNextIndex;
        }
        r->xEnd();

        float m[9];
        mreset(m);

        const float s[3] =
        {
            (fGraphWidth * 0.5f) * 0.33333334f,
            fHeight * 0.5f,
            0.0f,
        };

        const float v[3] =
        {
            fGraphWidth * 0.25f + x1,
            fHeight * 0.25f + y0,
            0.0f,
        };

        char szText[16];
        std::snprintf(szText, sizeof(szText), "%3d", lFPS);

        r->xDrawText(m, v, s, szText, static_cast<uint32_t>(std::strlen(szText)), 0x3FFFFF00u, 0);
    }
}
