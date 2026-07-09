#pragma once

#include <Glacier/ZHandle.h>
#include <BloodMoney/Game/UI/eZWUserEvents.h>


namespace Hitman::BloodMoney
{
    class ZWINDOWS;

    struct SWinKey 
    {
        Glacier::ZHandle* pHandle;
        uint32_t dwPrevState;
    };
    RE_VERIFY_SIZE(SWinKey, 0x8);

    struct SAnalogKey {
        // Size: 0x1c (28) bytes

        Glacier::ZHandle* pHandle;
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
        Glacier::ZHandle m_Down;
        Glacier::ZHandle m_Up;
        Glacier::ZHandle m_Left;
        Glacier::ZHandle m_Right;
        Glacier::ZHandle m_Select;
        Glacier::ZHandle m_Select2;
        Glacier::ZHandle m_Select3;
        Glacier::ZHandle m_Select4;
        Glacier::ZHandle m_Cancel;
        Glacier::ZHandle m_Misc1;
        Glacier::ZHandle m_MButton;
        Glacier::ZHandle m_LButton;
        Glacier::ZHandle m_RButton;
        Glacier::ZHandle m_MWheelUp;
        Glacier::ZHandle m_MWheelDown;
        Glacier::ZHandle m_Start;
        Glacier::ZHandle m_L1;
        Glacier::ZHandle m_R1;
        SWinKey m_aKeys[18];
        bool m_bKeyboardBinded;
        bool m_bInputHandlerInstalled;
        Glacier::ZHandle m_AnalogHorizontal;
        Glacier::ZHandle m_AnalogVertical;
        SAnalogKey m_aAnalogKeys[2];
    };
    // TODO: ^^^ Need verify size and check code ^^^
}