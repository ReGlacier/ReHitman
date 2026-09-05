#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Component/Component.h>
#include <Glacier/Component/ZComponent.h>
#include <Glacier/Component/ZGlobalComponentAllocator.h>
#include <Glacier/Component/ZComponentDescription.h>


namespace Glacier
{
    struct ZGlobalComponentBase : public ZComponent<ZGlobalComponentBase, ZComponentDescription<1, ZGlobalComponentAllocator>>
    {
        // vtbl
        ~ZGlobalComponentBase() override = default;
        
        virtual void NukeAndRestart()
        {
            // Do nothing
        }

        // methods
        ZGlobalComponentBase() = default;

        static ZComponentId ComponentId()
        { 
            return "ZGlobalComponentBase";
        }
    };
}