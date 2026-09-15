#include <Glacier/Input/ZInterface.h>


namespace Glacier
{
    SysInput::ZInterface::~ZInterface() = default;
    SysInput::ZInterface::ZInterface() = default;

    bool SysInput::ZInterface::OpenControllerMissingWindow()
    {
        return false;
    }

    bool SysInput::ZInterface::IsDialogAllowed()
    {
        return true;
    }

    void SysInput::ZInterface::SetIsDialogAllowed(bool)
    {
        // Do nothing here
    }

    bool SysInput::ZInterface::CheckForKeyPress(int* device, int* controlid)
    {
        return false;
    }
}