#include <Glacier/Input/ZMouseDevice.h>


namespace Glacier
{
    ZMouseDevice::~ZMouseDevice() = default;
    
    ZMouseDevice::ZMouseDevice(int digcount, int anacount)
        : ZInputDevice(SysInput::EDeviceType::eMOUSE_TYPE, digcount, anacount)
    {
    }

    bool ZMouseDevice::Connected()
    {
        return m_pDevice != nullptr;
    }
}