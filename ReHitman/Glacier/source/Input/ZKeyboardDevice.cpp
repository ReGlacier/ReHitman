#include <Glacier/Input/ZKeyboardDevice.h>

namespace Glacier
{
    ZKeyboardDevice::~ZKeyboardDevice() = default;

    bool ZKeyboardDevice::Connected()
    {
        return m_pDevice != nullptr;
    }
    
    ZKeyboardDevice::ZKeyboardDevice(int digcount, int anacount)
        : ZInputDevice(SysInput::EDeviceType::eKEYBOARD_TYPE, digcount, anacount)
    {
    }
}