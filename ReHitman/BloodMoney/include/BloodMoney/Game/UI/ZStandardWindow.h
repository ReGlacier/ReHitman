#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/zvector.h>
#include <Glacier/ZSTL/ZStaticVector.h>
#include <BloodMoney/Game/UI/IWindowInterface.h>

namespace Hitman::BloodMoney 
{
    class ZStandardWindow : public IWindowInterface 
    {
    public:
        //vtbl
        //data (total size is 0x15C, base size is 0x90)
        Glacier::zvector<IWindowInterface::ZElementExtraInfo> m_vecGUIElementExtraInfo;
        Glacier::ZStaticVector<IGUIElement*, 44> m_vecCurrentElements;
        int32_t m_iAlignmentId;
        bool m_bDialogWindow;
        bool m_bTRCWindow;
        int m_iTRCPriority;
    };
    RE_VERIFY_SIZE(ZStandardWindow, 0x15C); // Verified
}