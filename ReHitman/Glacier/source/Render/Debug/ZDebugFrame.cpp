#include <Glacier/Render/Debug/ZDebugFrame.h>


namespace Glacier
{
    ZDebugFrame::ZDebugFrame(ZDebugFrame* pParent)
        : ZDrawDebugRegion(pParent)
    {
        m_dwTextColor = 0xFFFFFFFFu;
        m_dwTextColorInv = 0xFF000000u;
        m_dwBgColorInv = 0xFFFFFFFFu;
    }
}
