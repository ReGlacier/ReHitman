#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Input/WintelFwd.h>
#include <Glacier/Input/ZInputDevice.h>


namespace Glacier
{
    class ZKeyboardDevice : public ZInputDevice
    {
    public:
        // vtbl
        ~ZKeyboardDevice() override;
        bool Connected() override;
        
        // methods
        ZKeyboardDevice(int digcount, int anacount);

        // members
        IDirectInput8A* m_pDI { nullptr };
        IDirectInputDevice8A* m_pDevice { nullptr };
    };
    RE_VERIFY_SIZE(ZKeyboardDevice, 0x74); // Verified
    RE_VERIFY_OFFSET(ZKeyboardDevice, m_pDI, 0x6C);
    RE_VERIFY_OFFSET(ZKeyboardDevice, m_pDevice, 0x70);
}