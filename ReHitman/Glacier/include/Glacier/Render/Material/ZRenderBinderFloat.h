#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/ZRenderBinder.h>


namespace Glacier
{
    class ZRenderBinderFloat : public ZRenderBinder
    {
    public:
        // vtbl
        virtual const float* GetValues() const = 0;

        // methods
        using ZRenderBinder::ZRenderBinder;

        // members
    };
}
