#include <Glacier/Input/ZGameControlDevice.h>


namespace Glacier
{
    ZGameControlDevice::~ZGameControlDevice() = default;

    bool ZGameControlDevice::Connected()
    {
        return false;
    }
    
    ZGameControlDevice::ZGameControlDevice(int digcount, int anacount)
        : ZInputDevice(SysInput::EDeviceType::eGAMECONTROL_TYPE, digcount, anacount)
    {
        m_fDeadZone = 0.27f;
    }
}
