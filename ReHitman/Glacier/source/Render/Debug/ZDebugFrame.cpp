#include <Glacier/Render/Debug/ZDebugFrame.h>
#include <Glacier/Render/Debug/ZDrawDebugText.h>
#include <Glacier/Render/Debug/Globals.h>

#include <cstdarg>
#include <cstdio>
#include <cstring>


namespace Glacier
{
    ZDebugFrame::ZDebugFrame(ZDebugFrame* pParent)
        : ZDrawDebugRegion(pParent)
    {
        m_dwTextColor = 0xFFFFFFFFu;
        m_dwTextColorInv = 0xFF000000u;
        m_dwBgColorInv = 0xFFFFFFFFu;
    }

    ZDebugFrame* ZDebugFrame::AddFrame()
    {
        return this;
    }

    void ZDebugFrame::SetTextColor(uint32_t lColor)
    {
        m_dwTextColor = lColor;
    }

    void ZDebugFrame::SetBgColorInv(uint32_t lColor)
    {
        m_dwBgColorInv = lColor;
    }

    void ZDebugFrame::DrawLine(float fStartX, float fStartY, float fEndX, float fEndY, uint32_t lColor)
    {
        g_pDrawDebugText->DrawLine(
            static_cast<float>(m_lStartX) + fStartX,
            static_cast<float>(m_lStartY) + fStartY,
            static_cast<float>(m_lStartX) + fEndX,
            static_cast<float>(m_lStartY) + fEndY,
            lColor);
    }

    int ZDebugFrame::Plot(uint32_t lLinePosX, uint32_t lLinePosY, const char* pFormat, ...)
    {
        char szText[1072];

        va_list args;
        va_start(args, pFormat);
        const int lResult = std::vsnprintf(szText, sizeof(szText), pFormat, args);
        va_end(args);

        g_pDrawDebugText->TextPlot(
            9 * lLinePosX + m_lStartX + 1,
            15 * lLinePosY + m_lStartY + 1,
            szText,
            m_dwTextColor);

        return lResult;
    }

    int ZDebugFrame::PlotInvers(uint32_t lLinePosX, uint32_t lLinePosY, const char* pFormat, ...)
    {
        char szText[1072];

        va_list args;
        va_start(args, pFormat);
        const int lResult = std::vsnprintf(szText, sizeof(szText), pFormat, args);
        va_end(args);

        g_pDrawDebugText->TextPlotInvers(
            9 * lLinePosX + m_lStartX + 1,
            15 * lLinePosY + m_lStartY + 1,
            szText,
            m_dwTextColorInv,
            m_dwBgColorInv);

        return lResult;
    }

    int ZDebugFrame::PlotFixed(uint32_t lLinePosX, uint32_t lLinePosY, const char* pText, const char* pFormat, ...)
    {
        char szValue[1024];

        va_list args;
        va_start(args, pFormat);
        const int lResult = std::vsnprintf(szValue, sizeof(szValue), pFormat, args);
        va_end(args);

        char szText[1072];
        std::strncpy(szText, pText, sizeof(szText) - 1);
        szText[sizeof(szText) - 1] = '\0';

        size_t lValueLen = std::strlen(szValue);
        const size_t lTextLen = std::strlen(szText);
        if (lValueLen > lTextLen)
        {
            szValue[lTextLen] = '\0';
            lValueLen = lTextLen;
        }

        std::memcpy(&szText[lTextLen - lValueLen], szValue, lValueLen + 1);

        g_pDrawDebugText->TextPlot(
            9 * lLinePosX + m_lStartX + 1,
            15 * lLinePosY + m_lStartY + 1,
            szText,
            0xFFFFFFFFu);

        return lResult;
    }

    int ZDebugFrame::PlotFixedInvers(uint32_t lLinePosX, uint32_t lLinePosY, const char* pText, const char* pFormat, ...)
    {
        char szValue[1024];

        va_list args;
        va_start(args, pFormat);
        const int lResult = std::vsnprintf(szValue, sizeof(szValue), pFormat, args);
        va_end(args);

        char szText[1072];
        std::strncpy(szText, pText, sizeof(szText) - 1);
        szText[sizeof(szText) - 1] = '\0';

        size_t lValueLen = std::strlen(szValue);
        const size_t lTextLen = std::strlen(szText);
        if (lValueLen > lTextLen)
        {
            szValue[lTextLen] = '\0';
            lValueLen = lTextLen;
        }

        std::memcpy(&szText[lTextLen - lValueLen], szValue, lValueLen + 1);

        g_pDrawDebugText->TextPlotInvers(
            9 * lLinePosX + m_lStartX + 1,
            15 * lLinePosY + m_lStartY + 1,
            szText,
            0xFF000000u,
            0xFFFFFFFFu);

        return lResult;
    }
}
