#include <Glacier/Render/Debug/ZDrawDebugRegion.h>
#include <Glacier/Render/Debug/ZDrawDebugText.h>
#include <Glacier/Render/Debug/Globals.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    ZDrawDebugRegion::ZDrawDebugRegion(ZDrawDebugRegion* pParent)
        : m_lBorderX(4)
        , m_lBorderY(4)
        , m_pParent(pParent)
    {
        if (pParent)
        {
            m_lStartX = pParent->m_lStartX;
            m_lStartY = pParent->m_lStartY;
            m_lSizeX = pParent->m_lSizeX;
            m_lSizeY = pParent->m_lSizeY;
        }
        else
        {
            m_lStartX = 0;
            m_lStartY = 0;
            m_lSizeX = 0;
            m_lSizeY = 0;
        }
    }

    void ZDrawDebugRegion::Set(int lStartX, int lStartY, int lSizeX, int lSizeY)
    {
        if (m_pParent)
        {
            int lEndX = lStartX + lSizeX;
            int lEndY = lStartY + lSizeY;

            if (lStartX < m_pParent->m_lStartX)
                lStartX = m_pParent->m_lStartX;
            if (lStartY < m_pParent->m_lStartY)
                lStartY = m_pParent->m_lStartY;

            const int lParentEndX = m_pParent->m_lSizeX + m_pParent->m_lStartX;
            const int lParentEndY = m_pParent->m_lSizeY + m_pParent->m_lStartY;

            if (lEndX > lParentEndX)
                lEndX = lParentEndX;
            if (lEndY > lParentEndY)
                lEndY = lParentEndY;

            lSizeX = lEndX - lStartX;
            lSizeY = lEndY - lStartY;
        }

        m_lStartX = lStartX;
        m_lStartY = lStartY;
        m_lSizeX = lSizeX;
        m_lSizeY = lSizeY;
    }

    void ZDrawDebugRegion::SetPosSize(int lStartX, int lStartY, int lSizeX, int lSizeY)
    {
        Set(m_lStartX + lStartX, m_lStartY + lStartY, lSizeX, lSizeY);
    }

    void ZDrawDebugRegion::DrawRect(int lPosX, int lPosY, int lSizeX, int lSizeY, uint32_t lColor)
    {
        g_pDrawDebugText->DrawRect(m_lStartX + lPosX, m_lStartY + lPosY, lSizeX, lSizeY, lColor);
    }

    void ZDrawDebugRegion::Fill(uint32_t lColor)
    {
        g_pDrawDebugText->DrawRect(
            m_lStartX - m_lBorderX,
            m_lStartY - m_lBorderY,
            2 * m_lBorderX + m_lSizeX,
            2 * m_lBorderY + m_lSizeY,
            lColor);
    }

    void ZDrawDebugRegion::DrawFocus()
    {
        g_pDrawDebugText->DrawRect(m_lStartX - m_lBorderX, m_lStartY - m_lBorderY,
            2 * m_lBorderX + m_lSizeX, 1, 0xFFFFFFFFu);
        g_pDrawDebugText->DrawRect(m_lStartX - m_lBorderX, m_lSizeY + m_lStartY + m_lBorderY - 1,
            2 * m_lBorderX + m_lSizeX, 1, 0xFFFFFFFFu);
        g_pDrawDebugText->DrawRect(m_lStartX - m_lBorderX, m_lStartY - m_lBorderY,
            1, 2 * m_lBorderY + m_lSizeY, 0xFFFFFFFFu);
        g_pDrawDebugText->DrawRect(m_lStartX + m_lBorderX + m_lSizeX - 1, m_lStartY - m_lBorderY,
            1, 2 * m_lBorderY + m_lSizeY, 0xFFFFFFFFu);
    }

    void ZDrawDebugRegion::SetPosSizeText(int lStartX, int lStartY, int lSizeX, int lSizeY)
    {
        Set(9 * lStartX + m_lStartX, 15 * lStartY + m_lStartY, 9 * lSizeX + 1, 15 * lSizeY + 1);
    }

    void ZDrawDebugRegion::SetSizeText(int lSizeX, int lSizeY)
    {
        Set(
            (m_lSizeX >> 1) - ((9 * lSizeX + 1) >> 1) + m_lStartX,
            (m_lSizeY >> 1) - ((15 * lSizeY + 1) >> 1) + m_lStartY,
            9 * lSizeX + 1,
            15 * lSizeY + 1);
    }

    void ZDrawDebugRegion::AlignPosSizeText(ZDrawDebugRegion::Align eAlign, int lStartX, int lStartY, int lSizeX, int lSizeY)
    {
        if (!m_pParent)
        {
            SetSizeText(lSizeX, lSizeY);
            return;
        }

        const int lWidth = 9 * lSizeX;
        const int lHeight = 15 * lSizeY;
        int lX;

        if (eAlign == LEFTTOP)
        {
            lX = 9 * lStartX;
        }
        else if (eAlign == RIGHTTOP)
        {
            lX = m_pParent->m_lSizeX + 9 * lStartX - lWidth;
        }
        else
        {
            ZASSERT(false);
            return;
        }

        Set(lX + m_lStartX, 15 * lStartY + m_lStartY, lWidth + 1, lHeight + 1);
    }
}
