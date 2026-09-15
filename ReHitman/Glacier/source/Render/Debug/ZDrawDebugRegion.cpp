#include <Glacier/Render/Debug/ZDrawDebugRegion.h>
#include <Glacier/Render/Debug/Globals.h>


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
}