#pragma once

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

        // members
        ZDrawSurface::TARGET m_lTarget;
    };
}