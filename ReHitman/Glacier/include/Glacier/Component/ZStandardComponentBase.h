#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Component/Component.h>
#include <Glacier/Component/ZComponent.h>
#include <Glacier/Component/ZComponentDescription.h>
#include <Glacier/Component/ZComponentDefaultAllocator.h>


namespace Glacier
{
    class ZStandardComponentBase : public ZComponent<ZStandardComponentBase, ZComponentDescription<0, ZComponentDefaultAllocator>>
    {
    public:
        // vtbl
        ~ZStandardComponentBase() override = default;

        // members
        static ZComponentId ComponentId() { return "StandardComponent"; }
    };
}