#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/ZRenderBinder.h>


namespace Glacier
{
    class ZRenderBinderInt : public ZRenderBinder
    {
    public:
        // vtbl
        virtual int32_t* GetValues() = 0;

        // methods
        using ZRenderBinder::ZRenderBinder;
    };
}
