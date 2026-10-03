#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Component/Component.h>
#include <Glacier/Component/ZComponent.h>
#include <Glacier/Component/ZComponentDescription.h>
#include <Glacier/Component/ZComponentDefaultAllocator.h>


namespace Glacier
{
    class ZRuntimeComponentBase : public ZComponent<ZRuntimeComponentBase, ZComponentDescription<0, ZComponentDefaultAllocator>>
    {
    public:
        // vtbl
        ~ZRuntimeComponentBase() override = default;

        virtual void NukeAndRestart()
        {
            // Do nothing
        }
        
        // methods
        static ZComponentId ComponentId() { return "ZRuntimeComponentBase"; }
    };
}