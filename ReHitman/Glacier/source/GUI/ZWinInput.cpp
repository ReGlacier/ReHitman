#include <Glacier/GUI/ZWinInput.h>
#include <Glacier/GUI/ZWINDOWS.h>
#include <Glacier/Input/EDeviceType.h>
#include <Glacier/Input/SysInput.h>
#include <Glacier/Input/ZDevice.h>
#include <Glacier/Input/ZInterface.h>
#include <Glacier/ZSTL/ZMath.h>


namespace Glacier
{
    namespace
    {
        const char* ZWINDOWSKEYS = "ZWINDOWSKEYS_1={ZW_DOWN=hold(kb,down);ZW_UP=hold(kb,up);ZW_LEFT=hold(kb,left);ZW_RIGHT=hold(kb,right);ZW_SELECT=hold(kb,return);ZW_CANCEL=hold(kb,esc);ZW_LBUTTON=hold(ms,left);ZW_RBUTTON=hold(ms,right);ZW_MBUTTON=hold(ms,middle);ZW_MWHEELUP=tap(ms,wheelup);ZW_MWHEELDOWN=tap(ms,wheeldown);};";
        Action::ZStaticBinds ZWINDOWSKEYSBinds(ZWINDOWSKEYS);

        int QueueInput(unsigned int iKey, int iState, void* pUserData)
        {
            ZWINDOWS* pWindows = static_cast<ZWINDOWS*>(pUserData);
            if (pWindows->m_iInputQuePos >= 5)
                return 0;

            pWindows->m_aiInputQue[pWindows->m_iInputQuePos++] = iKey | (iState << 16);
            return 1;
        }
    }

    ZWinInput::ZWinInput()
        : m_pWindows(nullptr)
        , m_Down("ZW_DOWN")
        , m_Up("ZW_UP")
        , m_Left("ZW_LEFT")
        , m_Right("ZW_RIGHT")
        , m_Select("ZW_SELECT")
        , m_Select2("ZW_SELECT2")
        , m_Select3("ZW_SELECT3")
        , m_Select4("ZW_SELECT4")
        , m_Cancel("ZW_CANCEL")
        , m_Misc1("ZW_MISC1")
        , m_MButton("ZW_MBUTTON")
        , m_LButton("ZW_LBUTTON")
        , m_RButton("ZW_RBUTTON")
        , m_MWheelUp("ZW_MWHEELUP")
        , m_MWheelDown("ZW_MWHEELDOWN")
        , m_Start("ZW_START")
        , m_L1("ZW_L1")
        , m_R1("ZW_R1")
        , m_bKeyboardBinded(false)
        , m_bInputHandlerInstalled(false)
    {
        Initialize();
    }

    ZWinInput::~ZWinInput()
    {
        if (m_bInputHandlerInstalled)
        {
            const int iKeyboard = SysInput::instance->GetPrimaryDevice(SysInput::EDeviceType::eKEYBOARD_TYPE);
            SysInput::instance->UninstallHandler(iKeyboard, QueueInput);
        }
    }

    void ZWinInput::Initialize()
    {
        Action::ZHandle* pHandles[eZW_CUSTOM_COUNT] =
        {
            &m_Down,
            &m_Up,
            &m_Left,
            &m_Right,
            &m_Select,
            &m_Select2,
            &m_Select3,
            &m_Select4,
            &m_Start,
            &m_L1,
            &m_R1,
            &m_Cancel,
            &m_Misc1,
            &m_LButton,
            &m_RButton,
            &m_MButton,
            &m_MWheelUp,
            &m_MWheelDown
        };

        for (int i = 0; i < eZW_CUSTOM_COUNT; ++i)
            m_aKeys[i].pHandle = pHandles[i];
    }

    void ZWinInput::Update()
    {
        if (!m_pWindows)
            return;

        if (!m_bKeyboardBinded)
            m_bKeyboardBinded = true;

        for (int i = 0; i < eZW_CUSTOM_COUNT; ++i)
        {
            const uint32_t dwState = m_aKeys[i].pHandle->Digital();
            if (m_aKeys[i].dwPrevState == dwState)
                continue;

            QueueInput(eZW_FIRST + i, dwState ? 0x200 : 0, m_pWindows);
            m_aKeys[i].dwPrevState = dwState;
        }

        const bool bWantKeyboardInput = m_pWindows->m_WindowStack.Count() > 0 && m_pWindows->m_pControlInFocus;
        if (bWantKeyboardInput && !m_bInputHandlerInstalled)
        {
            const int iKeyboard = SysInput::instance->GetPrimaryDevice(SysInput::EDeviceType::eKEYBOARD_TYPE);
            m_bInputHandlerInstalled = SysInput::instance->InstallHandler(iKeyboard, QueueInput, m_pWindows);
        }
        else if (!bWantKeyboardInput && m_bInputHandlerInstalled)
        {
            m_bInputHandlerInstalled = false;
            const int iKeyboard = SysInput::instance->GetPrimaryDevice(SysInput::EDeviceType::eKEYBOARD_TYPE);
            SysInput::instance->UninstallHandler(iKeyboard, QueueInput);
        }

        const int iMouse = SysInput::instance->GetPrimaryDevice(SysInput::EDeviceType::eMOUSE_TYPE);
        SysInput::ZDevice* pMouse = SysInput::instance->GetDevice(iMouse);
        if (pMouse)
        {
            ZVector2 vMouseDelta;
            vMouseDelta.x = pMouse->AnalogMotion(0);
            vMouseDelta.y = pMouse->AnalogMotion(1);
            m_pWindows->SetMousePosition(vMouseDelta);
            m_pWindows->m_bLeftMouseDown = pMouse->DigitalState(0) != 0;
        }
        else if (m_pWindows->m_bMouseActive)
        {
            m_pWindows->SetShowMouse(false);
        }
    }

    void ZWinInput::SetWindowsPtr(ZWINDOWS* pWindows)
    {
        m_pWindows = pWindows;
    }

    int ZWinInput::GetActivatedBy(int iKey)
    {
        return m_aKeys[iKey].pHandle->ActivatedBy();
    }
}
