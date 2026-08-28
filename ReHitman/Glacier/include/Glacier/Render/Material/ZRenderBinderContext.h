#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/ZRenderBinder.h>


namespace Glacier
{
    class ZRenderBinderContext : public ZRenderBinder
    {
    public:
        // vtbl
        virtual uint32_t GetContextType() const = 0;

        // methods
        using ZRenderBinder::ZRenderBinder;
    };
}
