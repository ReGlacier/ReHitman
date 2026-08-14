#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Render/Object/ZRenderObjectInstance.h>


namespace Glacier
{
    class ZRenderObjectInstanceX86 : public ZRenderObjectInstance
    {
    public:
        // constants
        static constexpr int MAX_VERTEX_BATCH = 0x10; // TODO: Verify with PC

        // vtbl
        ~ZRenderObjectInstanceX86() override;
        virtual void Draw(const ZMatrix&, const ZMatrix&, ZRender* pRender);

        // methods
        using ZRenderObjectInstance::ZRenderObjectInstance;
    };
}