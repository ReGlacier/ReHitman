#include <Glacier/Input/ZDevice.h>


namespace Glacier
{
    SysInput::ZDevice::ZDevice() = default;
    SysInput::ZDevice::~ZDevice() = default;

    const char* SysInput::ZDevice::DigitalSystemName(int id)
    {
        return DigitalName(id);
    }
}