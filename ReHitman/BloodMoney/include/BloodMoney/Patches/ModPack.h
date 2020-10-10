#pragma once

#include <HF/HackingFrameworkFWD.h>

namespace Hitman::BloodMoney
{
    /**
     * @struct ModPack
     * @brief Just a pack of modules who will be passed into Patch::Apply & Patch::Revert method
     */
    struct ModPack
    {
        HF::Win32::ProcessRef process;
        HF::Win32::ModuleRef self;
        HF::Win32::ModuleRef d3d9;
    };
}