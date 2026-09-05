#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/zvector.h>
#include <Glacier/ZSTL/ZStaticVector.h>
#include <Glacier/GUI/XMLInterface/Windows/IWindowInterface.h>


namespace Glacier
{
    class ZStandardWindow : public IWindowInterface
    {
    public:
        // vtbl
        // methods
        // members
        zvector<IWindowInterface::ZElementExtraInfo> m_vecGUIElementExtraInfo;
        ZStaticVector<IGUIElement*, 44> m_vecCurrentElements;
        int32_t m_iAlignmentId;
        bool m_bDialogWindow;
        bool m_bTRCWindow;
        int m_iTRCPriority;
    };
    RE_VERIFY_SIZE(ZStandardWindow, 0x15C); // Verified
}
