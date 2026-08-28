#pragma once

#include <Glacier/Action/ActionInterface.h>
#include <BloodMoney/Game/UI/eZWUserEvents.h>


namespace Hitman::BloodMoney
{
    class ZWINDOWS;

    struct SWinKey
    {
        Glacier::Action::ZHandle* pHandle;
        uint32_t dwPrevState;
    };
    RE_VERIFY_SIZE(SWinKey, 0x8);

    struct SAnalogKey {
        // Size: 0x1c (28) bytes

        Glacier::Action::ZHandle* pHandle;
        float fActivationTime;
        float fRepeatTime;
        eZWUserEvents eHighEvent;
        eZWUserEvents eLowEvent;
        bool bSentEventLastFrame;
        eZWUserEvents eLastEvent;
    };
    // TODO: ^^^ Need verify size of this struct ^^^

    class ZWinInput
    {
    public:
        // vtbl
        // data
        ZWINDOWS* m_pWindows;
        Glacier::Action::ZHandle m_Down;
        Glacier::Action::ZHandle m_Up;
        Glacier::Action::ZHandle m_Left;
        Glacier::Action::ZHandle m_Right;
        Glacier::Action::ZHandle m_Select;
        Glacier::Action::ZHandle m_Select2;
        Glacier::Action::ZHandle m_Select3;
        Glacier::Action::ZHandle m_Select4;
        Glacier::Action::ZHandle m_Cancel;
        Glacier::Action::ZHandle m_Misc1;
        Glacier::Action::ZHandle m_MButton;
        Glacier::Action::ZHandle m_LButton;
        Glacier::Action::ZHandle m_RButton;
        Glacier::Action::ZHandle m_MWheelUp;
        Glacier::Action::ZHandle m_MWheelDown;
        Glacier::Action::ZHandle m_Start;
        Glacier::Action::ZHandle m_L1;
        Glacier::Action::ZHandle m_R1;
        SWinKey m_aKeys[18];
        bool m_bKeyboardBinded;
        bool m_bInputHandlerInstalled;
        Glacier::Action::ZHandle m_AnalogHorizontal;
        Glacier::Action::ZHandle m_AnalogVertical;
        SAnalogKey m_aAnalogKeys[2];
    };
    // TODO: ^^^ Need verify size and check code ^^^
}
