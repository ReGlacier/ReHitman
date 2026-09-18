#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Input/ZGameControlDevice.h>
#include <Windows.h>
#include <dinput.h>
#include <cstdint>


namespace Glacier
{
    struct SGameControlCaps
    {
        uint32_t dwSize;
        uint8_t m_padding04[0x08];
        uint32_t dwAxes;
        uint32_t dwButtons;
        uint8_t m_padding14[0x18];
    };
    RE_VERIFY_SIZE(SGameControlCaps, 0x2C);

    class ZGameControlWintel : public ZGameControlDevice
    {
    public:
        // vtbl
        ~ZGameControlWintel() override;
        const char* DigitalSystemName(int id) override;
        bool Connected() override;
        bool Initialize() override;
        bool Terminate() override;
        void Update() override;
        int Acquire(int retries) override;
        int Acquire() override;
        int Unacquire() override;
        
        // methods
        ZGameControlWintel(IDirectInput8A* pDI, GUID guidInstance, const char* instanceName, const char* productName);
        void NormalizeStick(float* x, float* y);
        
        // members
        IDirectInput8A* m_pDI;                 // +0x70
        IDirectInputDevice8A* m_pDevice;       // +0x74
        GUID m_guidInstance;                   // +0x78
        bool m_bAcquired;                      // +0x88
        char m_instanceName[0x104];            // +0x89
        char m_productName[0x104];             // +0x18D
        int m_buttonCount;                     // +0x294
        int m_axisCount;                       // +0x298
        SGameControlCaps m_wintelCaps;         // +0x29C
    };
    RE_VERIFY_SIZE(ZGameControlWintel, 0x2C8);
    RE_VERIFY_OFFSET(ZGameControlWintel, m_pDI, 0x70);
    RE_VERIFY_OFFSET(ZGameControlWintel, m_pDevice, 0x74);
    RE_VERIFY_OFFSET(ZGameControlWintel, m_guidInstance, 0x78);
    RE_VERIFY_OFFSET(ZGameControlWintel, m_bAcquired, 0x88);
    RE_VERIFY_OFFSET(ZGameControlWintel, m_instanceName, 0x89);
    RE_VERIFY_OFFSET(ZGameControlWintel, m_productName, 0x18D);
    RE_VERIFY_OFFSET(ZGameControlWintel, m_buttonCount, 0x294);
    RE_VERIFY_OFFSET(ZGameControlWintel, m_axisCount, 0x298);
    RE_VERIFY_OFFSET(ZGameControlWintel, m_wintelCaps, 0x29C);
}
