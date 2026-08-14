#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Render/ZRenderBinder.h>


namespace Glacier
{
    class ZRenderBinderTexture : public ZRenderBinder
    {
    public:
        // vtbl
        ~ZRenderBinderTexture() override;
        virtual uint32_t GetTextureId() = 0;
    };
}
