#pragma once

#include <Glacier/Action/ActionInterface.h>
#include <Glacier/GUI/eZWUserEvents.h>


namespace Glacier
{
    class ZWINDOWS;

    struct SWinKey
    {
        Action::ZHandle* pHandle;
        uint32_t dwPrevState;
    };
    RE_VERIFY_SIZE(SWinKey, 0x8);

    class ZWinInput
    {
    public:
        // methods
        ZWinInput();
        ~ZWinInput();
        void Initialize();
        void Update();
        void SetWindowsPtr(ZWINDOWS* pWindows);
        int GetActivatedBy(int iKey);

        // members
        ZWINDOWS* m_pWindows;
        Action::ZHandle m_Down;
        Action::ZHandle m_Up;
        Action::ZHandle m_Left;
        Action::ZHandle m_Right;
        Action::ZHandle m_Select;
        Action::ZHandle m_Select2;
        Action::ZHandle m_Select3;
        Action::ZHandle m_Select4;
        Action::ZHandle m_Cancel;
        Action::ZHandle m_Misc1;
        Action::ZHandle m_MButton;
        Action::ZHandle m_LButton;
        Action::ZHandle m_RButton;
        Action::ZHandle m_MWheelUp;
        Action::ZHandle m_MWheelDown;
        Action::ZHandle m_Start;
        Action::ZHandle m_L1;
        Action::ZHandle m_R1;
        SWinKey m_aKeys[18];
        bool m_bKeyboardBinded;
        bool m_bInputHandlerInstalled;
        RE_ADD_PADDING(2);
    };
    RE_VERIFY_SIZE(ZWinInput, 0x170);

    RE_VERIFY_OFFSET(ZWinInput, m_aKeys, 0xDC);
    RE_VERIFY_OFFSET(ZWinInput, m_bKeyboardBinded, 0x16C);
    RE_VERIFY_OFFSET(ZWinInput, m_bInputHandlerInstalled, 0x16D);
}
