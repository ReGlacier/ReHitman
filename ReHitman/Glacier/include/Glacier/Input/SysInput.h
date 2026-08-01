#pragma once

#include <Glacier/ZUniMemory.h>
#include <Glacier/Input/Fwds.h>


namespace Glacier::SysInput
{
    STATIC_GLOBAL_CLASS_INSTANCE(SysInput::ZInterface*, instance);

    bool Initialize();
    void ShutDown();
}
