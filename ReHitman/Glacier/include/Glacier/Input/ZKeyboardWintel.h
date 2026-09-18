#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Input/ZKeyboardDevice.h>
#include <Glacier/ZUniAssert.h>
#include <Windows.h>
#include <dinput.h>
#include <cstdint>


namespace Glacier
{
    class ZKeyboardWintel : public ZKeyboardDevice
    {
    public:
        // vtbl
        ~ZKeyboardWintel() override;
        const char* DigitalSystemName(int id) override;
        bool Initialize() override;
        bool Terminate() override;
        void Update() override;
        int Acquire(int retries) override;
        int Acquire() override;
        int Unacquire() override;

        // methods
        ZKeyboardWintel(IDirectInput8A* pDI, GUID guidInstance, const char* instanceName, const char* productName);
        bool HasAtLeastOneHandler() const;
        bool PrepareDirectInputData(const DIDEVICEOBJECTDATA* pData, int* pKeyCode, int* pMode);
        void InvokeHandlers(int keyCode, int mode);

        // members
        GUID m_guidInstance;           // +0x74
        bool m_bAcquired;              // +0x84
        char m_instanceName[0x104];    // +0x85
        char m_productName[0x104];     // +0x189
        RE_ADD_PADDING(3);             // +0x28D..0x28F
        DIDEVICEOBJECTDATA m_keyEvents[256]; // +0x290
        uint16_t m_repeatKeyCode;      // +0x1690
        uint16_t m_repeatKeyMods;      // +0x1692
        uint32_t m_repeatNextTime;     // +0x1694
    };
    RE_VERIFY_SIZE(ZKeyboardWintel, 0x1698);
    RE_VERIFY_OFFSET(ZKeyboardWintel, m_guidInstance, 0x74);
    RE_VERIFY_OFFSET(ZKeyboardWintel, m_bAcquired, 0x84);
    RE_VERIFY_OFFSET(ZKeyboardWintel, m_instanceName, 0x85);
    RE_VERIFY_OFFSET(ZKeyboardWintel, m_productName, 0x189);
    RE_VERIFY_OFFSET(ZKeyboardWintel, m_keyEvents, 0x290);
    RE_VERIFY_OFFSET(ZKeyboardWintel, m_repeatKeyCode, 0x1690);
    RE_VERIFY_OFFSET(ZKeyboardWintel, m_repeatKeyMods, 0x1692);
    RE_VERIFY_OFFSET(ZKeyboardWintel, m_repeatNextTime, 0x1694);

}
