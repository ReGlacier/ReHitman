#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Input/ZMouseDevice.h>
#include <Glacier/Input/WintelFwd.h>
#include <Glacier/ZUniMemory.h>
#include <Windows.h>
#include <cstdint>


namespace Glacier
{
    struct SMouseData
    {
        LONG lX;             // +0x00
        LONG lY;             // +0x04
        LONG lZ;             // +0x08 wheel
        BYTE rgbButtons[8];  // +0x0C..0x13
    };
    RE_VERIFY_SIZE(SMouseData, 0x14);

    class ZMouseWintel : public ZMouseDevice
    {
    public:
        // vtbl
        ~ZMouseWintel() override;
        const char* DigitalSystemName(int id) override;
        bool Initialize() override;
        bool Terminate() override;
        void Update() override;
        int Acquire(int retries) override;
        int Acquire() override;
        int Unacquire() override;

        // methods
        ZMouseWintel(
            IDirectInput8A* pDI, 
            GUID guidInstance,
            const char* instanceName,
            const char* productName
        );

        // members
        GUID m_guidInstance;                // +0x74
        bool m_bAcquired;                   // +0x84
        char m_instanceName[0x104];         // +0x85
        char m_productName[0x104];          // +0x189
        RE_ADD_PADDING(3);                  // +0x28D..0x28F
        SMouseData m_mouseData;             // +0x290
    };
    RE_VERIFY_SIZE(ZMouseWintel, 0x2A4);
    RE_VERIFY_OFFSET(ZMouseWintel, m_guidInstance, 0x74);
    RE_VERIFY_OFFSET(ZMouseWintel, m_bAcquired, 0x84);
    RE_VERIFY_OFFSET(ZMouseWintel, m_instanceName, 0x85);
    RE_VERIFY_OFFSET(ZMouseWintel, m_productName, 0x189);
    RE_VERIFY_OFFSET(ZMouseWintel, m_mouseData, 0x290);

    STATIC_GLOBAL_CLASS_INSTANCE(ZInputDevice::CtrlInfo*, g_MouseDigitalControls);
    STATIC_GLOBAL_CLASS_INSTANCE(ZInputDevice::CtrlInfo*, g_MouseAnalogControls);
}
