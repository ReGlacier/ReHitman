#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Input/ZInputDevice.h>
#include <Glacier/Input/WintelFwd.h>
#include <cstdint>


namespace Glacier
{
    class ZMouseDevice : public ZInputDevice
    {
    public:
        // vtbl
        ~ZMouseDevice() override;
        bool Connected() override;

        // methods
        ZMouseDevice(int digcount, int anacount);

        // members
        IDirectInput8A* m_pDI { nullptr };
        IDirectInputDevice8A* m_pDevice { nullptr };
    };

    RE_VERIFY_SIZE(ZMouseDevice, 0x74); // Verified
    RE_VERIFY_OFFSET(ZMouseDevice, m_pDI, 0x6C);
    RE_VERIFY_OFFSET(ZMouseDevice, m_pDevice, 0x70);
}