#include <Glacier/Input/ZKeyboardWintel.h>
#include <Glacier/System/ZSysInterface.h>

#include <cstring>

#ifndef DIDFT_OPTIONAL
static constexpr DWORD DIDFT_OPTIONAL = 0x80000000;
#endif


namespace Glacier
{
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(uint8_t, g_bKeyboardGraveAccent, 0x008B4DB0, 0);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(uint8_t, g_bKeyboardAcuteAccent, 0x008B4DB1, 0);
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(uint8_t, g_bKeyboardUmlautAccent, 0x008B4DB2, 0);

    static DIOBJECTDATAFORMAT s_KeyboardObjectDataFormat[256];

    static DIDATAFORMAT g_KeyboardDataFormat =
    {
        sizeof(DIDATAFORMAT),
        sizeof(DIOBJECTDATAFORMAT),
        DIDF_RELAXIS,
        256,
        256,
        s_KeyboardObjectDataFormat
    };

    static void InitKeyboardDataFormat()
    {
        static bool initialized = false;
        if (initialized)
        {
            return;
        }

        for (int i = 0; i < 256; ++i)
        {
            s_KeyboardObjectDataFormat[i].pguid = &GUID_Key;
            s_KeyboardObjectDataFormat[i].dwOfs = i;
            s_KeyboardObjectDataFormat[i].dwType = DIDFT_PSHBUTTON | DIDFT_OPTIONAL | DIDFT_MAKEINSTANCE(i);
            s_KeyboardObjectDataFormat[i].dwFlags = 0;
        }

        initialized = true;
    }

    static char s_ZKeyboardDigitalSystemNameBuffer[260];

    static ZInputDevice::CtrlInfo s_KeyboardDigitalControls[] =
    {
        { .name = "esc", .nr = 1 }, // 0x0075F648 aEsc
        { .name = "escape", .nr = 1 }, // 0x0075F640 aEscape_0
        { .name = "1", .nr = 2 }, // 0x0075DDD8 a1
        { .name = "2", .nr = 3 }, // 0x0075AF70 zIkLnkObjPtr
        { .name = "3", .nr = 4 }, // 0x0075F69C a3
        { .name = "4", .nr = 5 }, // 0x0075F694 pChunkFile
        { .name = "5", .nr = 6 }, // 0x0075F68C a5
        { .name = "6", .nr = 7 }, // 0x0075F684 a6
        { .name = "7", .nr = 8 }, // 0x0075F67C a7
        { .name = "8", .nr = 9 }, // 0x0075F674 a8
        { .name = "9", .nr = 10 }, // 0x0075F63C a9
        { .name = "0", .nr = 11 }, // 0x0075F638 a0_0
        { .name = "backspace", .nr = 14 }, // 0x0075F62C aBackspace
        { .name = "bckspc", .nr = 14 }, // 0x0075F624 aBckspc
        { .name = "back", .nr = 14 }, // 0x0075F61C aBack_0
        { .name = "tab", .nr = 15 }, // 0x0075F618
        { .name = "q", .nr = 16 }, // 0x0075AC3C unk_75AC3C
        { .name = "w", .nr = 17 }, // 0x0075AC18 aW
        { .name = "e", .nr = 18 }, // 0x0075D1C8 unk_75D1C8
        { .name = "r", .nr = 19 }, // 0x00757200 aR
        { .name = "t", .nr = 20 }, // 0x0075CEAC aT_0
        { .name = "y", .nr = 21 }, // 0x0075B1D8 aY_0
        { .name = "u", .nr = 22 }, // 0x0075ADB0 unk_75ADB0
        { .name = "i", .nr = 23 }, // 0x0075C904 unk_75C904
        { .name = "o", .nr = 24 }, // 0x0075D118 unk_75D118
        { .name = "p", .nr = 25 }, // 0x0075B6F8 aP_0
        { .name = "return", .nr = 28 }, // 0x0075F610 aReturn
        { .name = "lctrl", .nr = 29 }, // 0x0075F608 aLctrl
        { .name = "a", .nr = 30 }, // 0x0075D0E8 unk_75D0E8
        { .name = "s", .nr = 31 }, // 0x0075BE68 unk_75BE68
        { .name = "d", .nr = 32 }, // 0x0075C8F0 unk_75C8F0
        { .name = "f", .nr = 33 }, // 0x0075D33C unk_75D33C
        { .name = "g", .nr = 34 }, // 0x0075B38C aG
        { .name = "h", .nr = 35 }, // 0x0075CDB4 asc_75CDB4
        { .name = "j", .nr = 36 }, // 0x0075F604 aJ
        { .name = "k", .nr = 37 }, // 0x0075AF90 unk_75AF90
        { .name = "l", .nr = 38 }, // 0x0075CE84 unk_75CE84
        { .name = "lshift", .nr = 42 }, // 0x0075F5FC aLshift
        { .name = "z", .nr = 44 }, // 0x0075F654 aZ
        { .name = "x", .nr = 45 }, // 0x0075B42C asc_75B42C
        { .name = "c", .nr = 46 }, // 0x0075CDF0 unk_75CDF0
        { .name = "v", .nr = 47 }, // 0x0075ADFC aV
        { .name = "b", .nr = 48 }, // 0x0075CD90 unk_75CD90
        { .name = "n", .nr = 49 }, // 0x0075AFE4 aN
        { .name = "m", .nr = 50 }, // 0x0075D12C unk_75D12C
        { .name = ",", .nr = 51 }, // 0x0075F5F8 asc_75F5F8
        { .name = ".", .nr = 52 }, // 0x00756854 a_
        { .name = "rshift", .nr = 54 }, // 0x0075F5F0 aRshift
        { .name = "num*", .nr = 55 }, // 0x0075F5E8 aNum_3
        { .name = "lalt", .nr = 56 }, // 0x0075F5E0 aLalt
        { .name = "space", .nr = 57 }, // 0x0075F5D8 aSpace
        { .name = "spc", .nr = 57 }, // 0x0075F5D4
        { .name = "cpslock", .nr = 58 }, // 0x0075F5CC aCpslock
        { .name = "f1", .nr = 59 }, // 0x0075F5C8 aF1
        { .name = "f2", .nr = 60 }, // 0x0075F5C4 aF2
        { .name = "f3", .nr = 61 }, // 0x0075F5C0 aF3
        { .name = "f4", .nr = 62 }, // 0x0075F5BC aF4
        { .name = "f5", .nr = 63 }, // 0x0075F5B8 aF5
        { .name = "f6", .nr = 64 }, // 0x0075F5B4 aF6
        { .name = "f7", .nr = 65 }, // 0x0075F5B0 aF7
        { .name = "f8", .nr = 66 }, // 0x0075F5AC aF8
        { .name = "f9", .nr = 67 }, // 0x0075F5A8 aF9
        { .name = "f10", .nr = 68 }, // 0x0075F5A4 aF10
        { .name = "numlock", .nr = 69 }, // 0x0075F59C aNumlock
        { .name = "scroll", .nr = 70 }, // 0x0075F594 aScroll
        { .name = "num7", .nr = 71 }, // 0x0075F58C aNum7
        { .name = "num8", .nr = 72 }, // 0x0075F584 aNum8
        { .name = "num9", .nr = 73 }, // 0x0075F57C aNum9
        { .name = "num-", .nr = 74 }, // 0x0075F574 aNum_2
        { .name = "num4", .nr = 75 }, // 0x0075F56C aNum4
        { .name = "num5", .nr = 76 }, // 0x0075F564 aNum5
        { .name = "num6", .nr = 77 }, // 0x0075F55C aNum6
        { .name = "num+", .nr = 78 }, // 0x0075F554 aNum_1
        { .name = "num1", .nr = 79 }, // 0x0075F54C aNum1
        { .name = "num2", .nr = 80 }, // 0x0075F544 aNum2
        { .name = "num3", .nr = 81 }, // 0x0075F53C aNum3
        { .name = "num0", .nr = 82 }, // 0x0075F534 aNum0
        { .name = "f11", .nr = 87 }, // 0x0075F530 aF11
        { .name = "f12", .nr = 88 }, // 0x0075F52C aF12
        { .name = "f13", .nr = 100 }, // 0x0075F528 aF13
        { .name = "f14", .nr = 101 }, // 0x0075F524 aF14
        { .name = "f15", .nr = 102 }, // 0x0075F520 aF15
        { .name = "numenter", .nr = 156 }, // 0x0075F514 aNumenter
        { .name = "rctrl", .nr = 157 }, // 0x0075F50C aRctrl
        { .name = "num,", .nr = 179 }, // 0x0075F504 aNum_0
        { .name = "num/", .nr = 181 }, // 0x0075F4FC aNum
        { .name = "num,", .nr = 83 }, // 0x0075F504 aNum_0
        { .name = "numdel", .nr = 83 }, // 0x0075F4F4 aNumdel
        { .name = "sysrq", .nr = 183 }, // 0x0075F4EC aSysrq
        { .name = "ralt", .nr = 184 }, // 0x0075F4E4 aRalt
        { .name = "home", .nr = 199 }, // 0x0075F4DC aHome
        { .name = "up", .nr = 200 }, // 0x0075F6E4 aUp
        { .name = "prev", .nr = 201 }, // 0x0075F4D4 aPrev
        { .name = "pgup", .nr = 201 }, // 0x0075F4CC aPgup
        { .name = "left", .nr = 203 }, // 0x0075F6D4 aLeft
        { .name = "right", .nr = 205 }, // 0x0075F6DC aRight
        { .name = "end", .nr = 207 }, // 0x0075F4C8 aEnd
        { .name = "down", .nr = 208 }, // 0x0075C388 aDown
        { .name = "next", .nr = 209 }, // 0x0075F4C0 aNext
        { .name = "pgdn", .nr = 209 }, // 0x0075F4B8 aPgdn
        { .name = "ins", .nr = 210 }, // 0x0075F4B4 aIns
        { .name = "del", .nr = 211 }, // 0x0075F4B0 aDel
        { .name = "lwin", .nr = 219 }, // 0x0075F4A8 aLwin
        { .name = "rwin", .nr = 220 }, // 0x0075F4A0 aRwin
        { .name = "aps", .nr = 221 }, // 0x0075F49C aAps
        { .name = "pause", .nr = 197 }, // 0x0075F494 aPause_0
        { .name = "num=", .nr = 141 }, // 0x0075F48C aNum_4
        { .name = "tilde", .nr = 41 }, // 0x0075F484 aTilde
        { .name = "grave", .nr = 41 }, // 0x0075F47C aGrave_0
        { .name = "lbracket", .nr = 26 }, // 0x0075F470 aLbracket
        { .name = "aa", .nr = 26 }, // 0x0075F46C aAa
        { .name = "rbracket", .nr = 27 }, // 0x0075F460 aRbracket
        { .name = "semicolon", .nr = 39 }, // 0x0075F454 aSemicolon
        { .name = "ae", .nr = 39 }, // 0x0075AF98 unk_75AF98
        { .name = "oe", .nr = 40 }, // 0x0075F450 aOe
        { .name = "-", .nr = 12 }, // 0x00756850 asc_756850
        { .name = "=", .nr = 13 }, // 0x0075E29C asc_75E29C
        { .name = "apostrophe", .nr = 43 }, // 0x0075F444 aApostrophe
        { .name = "slash", .nr = 53 }, // 0x0075F43C aSlash
        { .name = "lessthan", .nr = 86 }, // 0x0075F430 aLessthan
        { .name = "lt", .nr = 86 }, // 0x0075AD7C unk_75AD7C
        {},
    }; // at 0x0075F060
    
    // ZKeyboardWintel
    ZKeyboardWintel::ZKeyboardWintel(IDirectInput8A* pDI, GUID guidInstance, const char* instanceName, const char* productName)
        : ZKeyboardDevice(256, 0)
    {
        m_pDI = nullptr;
        m_pDevice = nullptr;
        m_bAcquired = false;
        m_instanceName[0] = '\0';
        m_productName[0] = '\0';
        m_repeatNextTime = 0;

        std::strncpy(m_instanceName, instanceName, sizeof(m_instanceName));
        std::strncpy(m_productName, productName, sizeof(m_productName));
        m_instanceName[sizeof(m_instanceName) - 1] = '\0';
        m_productName[sizeof(m_productName) - 1] = '\0';

        m_guidInstance = guidInstance;
        m_pDI = pDI;
        m_repeatKeyCode = 0;
        m_repeatKeyMods = 0;
        m_repeatNextTime = 0;
        m_diginf = s_KeyboardDigitalControls;
    }

    ZKeyboardWintel::~ZKeyboardWintel()
    {
        Terminate();
    }

    const char* ZKeyboardWintel::DigitalSystemName(int id)
    {
        if (!m_pDevice)
        {
            return nullptr;
        }

        DIPROPSTRING data{};
        data.diph.dwSize = sizeof(DIPROPSTRING);
        data.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        data.diph.dwObj = id;
        data.diph.dwHow = DIPH_BYOFFSET;

        m_pDevice->GetProperty(DIPROP_KEYNAME, &data.diph);

        char* output = s_ZKeyboardDigitalSystemNameBuffer;
        const WCHAR* input = data.wsz;
        do
        {
            *output++ = static_cast<char>(*input);
        }
        while (*input++);

        return s_ZKeyboardDigitalSystemNameBuffer;
    }

    bool ZKeyboardWintel::Initialize()
    {
        InitKeyboardDataFormat();

        if (FAILED(m_pDI->CreateDevice(m_guidInstance, &m_pDevice, nullptr)))
        {
            return false;
        }

        if (FAILED(m_pDevice->SetDataFormat(&g_KeyboardDataFormat)))
        {
            return false;
        }

        if (FAILED(m_pDevice->SetCooperativeLevel(static_cast<HWND>(g_pSysInterface->MainhWnd), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
        {
            return false;
        }

        DIPROPDWORD bufferSize{};
        bufferSize.diph.dwSize = sizeof(DIPROPDWORD);
        bufferSize.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        bufferSize.diph.dwObj = 0;
        bufferSize.diph.dwHow = DIPH_DEVICE;
        bufferSize.dwData = 256;

        if (FAILED(m_pDevice->SetProperty(DIPROP_BUFFERSIZE, &bufferSize.diph)))
        {
            return false;
        }

        Unacquire();
        return true;
    }

    bool ZKeyboardWintel::Terminate()
    {
        if (!m_pDevice)
        {
            return false;
        }

        m_pDevice->Unacquire();
        m_pDevice->Release();
        m_pDevice = nullptr;
        m_bAcquired = false;
        return true;
    }

    void ZKeyboardWintel::Update()
    {
        DWORD eventCount = 256;

        if (!m_pDevice || !m_active)
        {
            return;
        }

        if (!m_bAcquired)
        {
            Acquire();
        }

        if (FAILED(m_pDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), m_keyEvents, &eventCount, 0)))
        {
            m_bAcquired = false;
            return;
        }

        if (m_digcount > 0)
        {
            std::memset(m_pbDigital, 0, m_digcount);
        }

        if (eventCount)
        {
            m_repeatNextTime = 0;
        }

        if (m_updatetime != g_pSysInterface->m_fRealTime)
        {
            m_updatetime = g_pSysInterface->m_fRealTime;
        }

        const bool rShiftDown = GetAsyncKeyState(VK_RSHIFT) != 0;
        if (m_pbDigital[DIK_RSHIFT] != rShiftDown)
        {
            buttonSample(DIK_RSHIFT, rShiftDown, g_pSysInterface->m_fRealTime);
        }

        for (DWORD i = 0; i < eventCount; ++i)
        {
            const DIDEVICEOBJECTDATA& event = m_keyEvents[i];
            const int key = static_cast<int>(event.dwOfs);
            const bool pressed = (event.dwData & 0x80) != 0;

            if (key == DIK_RSHIFT)
            {
                continue;
            }

            buttonSample(key, pressed, g_pSysInterface->m_fRealTime);

            if (!HasAtLeastOneHandler())
            {
                continue;
            }

            int keyCode = 0;
            int keyMods = 0;
            if (!PrepareDirectInputData(&event, &keyCode, &keyMods))
            {
                if (keyMods == 0x200)
                {
                    const char* keyName = DigitalSystemName(key);
                    if (!stricmp(keyName, "GRAVE"))
                    {
                        g_bKeyboardGraveAccent = 1;
                    }
                    else if (!stricmp(keyName, "AGUDO"))
                    {
                        g_bKeyboardAcuteAccent = 1;
                    }
                }

                if ((keyMods & 0x200) && (keyMods & 3))
                {
                    const char* keyName = DigitalSystemName(key);
                    if (!stricmp(keyName, "AGUDO"))
                    {
                        g_bKeyboardUmlautAccent = 1;
                    }
                }

                continue;
            }

            if (pressed)
            {
                m_repeatKeyCode = static_cast<uint16_t>(keyCode);
                m_repeatKeyMods = static_cast<uint16_t>(keyMods);
                m_repeatNextTime = m_updatetime.secs + 204;
            }
            else
            {
                m_repeatNextTime = 0;
            }

            if (g_bKeyboardUmlautAccent)
            {
                if (static_cast<unsigned char>(keyCode) == 'U')
                {
                    keyCode = 220;
                }
                else if (static_cast<unsigned char>(keyCode) == 'u')
                {
                    keyCode = 252;
                }
            }
            else if (g_bKeyboardAcuteAccent)
            {
                switch (static_cast<char>(keyCode))
                {
                case 'A': keyCode = 193; break;
                case 'E': keyCode = 201; break;
                case 'I': keyCode = 205; break;
                case 'O': keyCode = 211; break;
                case 'U': keyCode = 218; break;
                case 'a': keyCode = 225; break;
                case 'e': keyCode = 233; break;
                case 'i': keyCode = 237; break;
                case 'o': keyCode = 243; break;
                case 'u': keyCode = 250; break;
                default: break;
                }
            }
            else if (g_bKeyboardGraveAccent)
            {
                switch (static_cast<char>(keyCode))
                {
                case 'A': keyCode = 192; break;
                case 'E': keyCode = 200; break;
                case 'I': keyCode = 204; break;
                case 'O': keyCode = 210; break;
                case 'U': keyCode = 217; break;
                case 'a': keyCode = 224; break;
                case 'e': keyCode = 232; break;
                case 'i': keyCode = 236; break;
                case 'o': keyCode = 242; break;
                case 'u': keyCode = 249; break;
                default: break;
                }
            }

            g_bKeyboardGraveAccent = 0;
            g_bKeyboardAcuteAccent = 0;
            g_bKeyboardUmlautAccent = 0;

            InvokeHandlers(keyCode, keyMods);
        }

        if (HasAtLeastOneHandler() && m_repeatNextTime && m_updatetime.secs > static_cast<int>(m_repeatNextTime))
        {
            m_repeatNextTime = m_updatetime.secs + 30;
            InvokeHandlers(static_cast<int16_t>(m_repeatKeyCode), static_cast<int16_t>(m_repeatKeyMods));
        }
    }

    int ZKeyboardWintel::Acquire(int retries)
    {
        if (!m_pDevice)
        {
            return 0;
        }

        for (int i = retries; i >= 0; --i)
        {
            const HRESULT hr = m_pDevice->Acquire();
            if (hr == DI_OK || hr == S_FALSE)
            {
                m_bAcquired = true;
                return 1;
            }
        }

        return 0;
    }

    int ZKeyboardWintel::Acquire()
    {
        return Acquire(0);
    }

    int ZKeyboardWintel::Unacquire()
    {
        if (!m_pDevice)
        {
            return 0;
        }

        m_pDevice->Unacquire();
        m_bAcquired = false;
        return 1;
    }

    bool ZKeyboardWintel::HasAtLeastOneHandler() const
    {
        for (int i = 0; i < 2; ++i)
        {
            if (m_vHandlers[i])
            {
                return true;
            }
        }

        return false;
    }

    bool ZKeyboardWintel::PrepareDirectInputData(const DIDEVICEOBJECTDATA* pData, int* pKeyCode, int* pMode)
    {
        *pMode = 0;
        if (pData->dwData & 0x80)
        {
            *pMode = 0x200;
        }

        if (m_pbDigital[DIK_LSHIFT]) *pMode |= 0x01;
        if (m_pbDigital[DIK_RSHIFT]) *pMode |= 0x02;
        if (m_pbDigital[DIK_LCONTROL]) *pMode |= 0x08;
        if (m_pbDigital[DIK_RCONTROL]) *pMode |= 0x10;
        if (m_pbDigital[DIK_LMENU]) *pMode |= 0x40;
        if (m_pbDigital[DIK_RMENU]) *pMode |= 0x80;

        switch (pData->dwOfs)
        {
        case DIK_GRAVE: *pKeyCode = 0x0A; return true;
        case DIK_F1: *pKeyCode = 0x0F; return true;
        case DIK_F2: *pKeyCode = 0x10; return true;
        case DIK_F3: *pKeyCode = 0x11; return true;
        case DIK_F4: *pKeyCode = 0x12; return true;
        case DIK_F5: *pKeyCode = 0x13; return true;
        case DIK_F6: *pKeyCode = 0x14; return true;
        case DIK_F7: *pKeyCode = 0x15; return true;
        case DIK_F8: *pKeyCode = 0x16; return true;
        case DIK_F9: *pKeyCode = 0x17; return true;
        case DIK_F10: *pKeyCode = 0x18; return true;
        case DIK_F11: *pKeyCode = 0x19; return true;
        case DIK_F12: *pKeyCode = 0x1A; return true;
        case DIK_HOME: *pKeyCode = 3; return true;
        case DIK_UP: *pKeyCode = 5; return true;
        case DIK_PRIOR: *pKeyCode = 0; return true;
        case DIK_LEFT: *pKeyCode = 7; return true;
        case DIK_RIGHT: *pKeyCode = 8; return true;
        case DIK_END: *pKeyCode = 4; return true;
        case DIK_DOWN: *pKeyCode = 6; return true;
        case DIK_NEXT: *pKeyCode = 1; return true;
        case DIK_INSERT: *pKeyCode = 0x1E; return true;
        case DIK_DELETE: *pKeyCode = 0x0B; return true;
        default:
            break;
        }

        const HKL keyboardLayout = GetKeyboardLayout(0);
        const UINT virtualKey = MapVirtualKeyExA(pData->dwOfs, MAPVK_VSC_TO_VK, keyboardLayout);
        switch (virtualKey)
        {
        case VK_BACK: *pKeyCode = 0x0C; return true;
        case VK_TAB: *pKeyCode = 0x0E; return true;
        case VK_RETURN: *pKeyCode = 9; return true;
        case VK_END: *pKeyCode = 4; return true;
        case VK_HOME: *pKeyCode = 3; return true;
        case VK_OEM_5: *pKeyCode = 0x0A; return true;
        default:
            break;
        }

        BYTE keyState[256];
        GetKeyboardState(keyState);

        WORD ascii = 0;
        if (ToAsciiEx(virtualKey, pData->dwOfs, keyState, &ascii, 0, keyboardLayout) == 1)
        {
            *pKeyCode = ascii;
            return ascii >= 32;
        }

        return false;
    }

    void ZKeyboardWintel::InvokeHandlers(int keyCode, int mode)
    {
        for (int i = 0; i < 2; ++i)
        {
            if (m_vHandlers[i])
            {
                m_vHandlers[i](keyCode, mode, m_vHandlerArgs[i]);
            }
        }
    }

}
