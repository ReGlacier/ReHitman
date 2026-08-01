#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Input/ZInputDevice.h>


namespace Glacier
{
    class ZGameControlDevice : public ZInputDevice
    {
    public:
        // vtbl
        ~ZGameControlDevice() override;
        bool Connected() override;
        
        // methods
        ZGameControlDevice(int digcount, int anacount);

        // members
        float m_fDeadZone { 0.27f };
    };
    RE_VERIFY_SIZE(ZGameControlDevice, 0x70); // Verified
    RE_VERIFY_OFFSET(ZGameControlDevice, m_fDeadZone, 0x6C);

}
