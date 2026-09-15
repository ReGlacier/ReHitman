#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/Object/ZRenderObject.h>


namespace Glacier
{
    class ZRenderObjectX86 : public ZRenderObject
    {
    public:
        // vtbl
        ~ZRenderObjectX86() override = default;

        // methods
        using ZRenderObject::ZRenderObject;
    };
}