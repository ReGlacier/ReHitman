#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    struct ZDrawSurface
    {
        enum TARGET : int
        {
            TEXTURE = 0x0,
            SCREEN = 0x1
        };

        // vtbl
        virtual ~ZDrawSurface();
        virtual void Begin();
        virtual void End();
        virtual void SetViewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
        virtual void FreeDeviceBuffers();
        virtual void AllocDeviceBuffers();

        // methods
        ZDrawSurface(ZDrawSurface::TARGET lTarget);

        // members
        ZDrawSurface::TARGET m_lTarget;
    };
    RE_VERIFY_SIZE(ZDrawSurface, 8); // verified PC ZRenderViewBase::CreateSurface
}