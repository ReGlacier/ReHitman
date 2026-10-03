#include <Glacier/Input/SysInput.h>
#include <Glacier/Input/ZInterface.h>
#include <Glacier/Input/ZSysInputWintel.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier::SysInput
{
    bool Initialize()
    {
        SysInput::instance = ZUniMemory::New<ZSysInputWintel>();
        return SysInput::instance != nullptr;
    }

    void ShutDown()
    {
        if (instance)
        {
            ZUniMemory::Delete(instance);
        }

        instance = nullptr;
    }


    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(SysInput::ZInterface*, instance, 0x008ACAA0, nullptr);
}