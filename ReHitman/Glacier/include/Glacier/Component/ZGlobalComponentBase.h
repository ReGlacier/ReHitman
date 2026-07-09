#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Component/ZComponentDescription.h>


namespace Glacier
{
    struct ZGlobalComponentBase : public ZComponentDefaultAllocatorDescription<ZGlobalComponentBase>
    {
        virtual void NukeAndRestart();
    };
}