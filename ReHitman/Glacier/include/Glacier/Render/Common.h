#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    struct _GLC_RENDER_RESOLUTION
    {
        // types
        enum EASPECT
        {
            Aspect4_3 = 0x0,
            Aspect5_4 = 0x1,
            Aspect16_9 = 0x2,
            Aspect16_10 = 0x3,
        };

        // members
        uint32_t dwSizeX { 0 };
        uint32_t dwSizeY { 0 };
        uint32_t dwBitsPerPixel { 0 };
        uint32_t bFullScreen { 0 };
        EASPECT eAspect { EASPECT::Aspect4_3 };
    };

    struct _GLC_RENDERCAPS
    {
        uint32_t dwCaps;
        uint32_t dwMultiSample;
    };

    struct SZVRECT
    {
        int x1;
        int y1;
        int x2;
        int y2;
    };

    struct SSpriteArrayBox;
    struct SVertexWintel;
    enum _GLC_PRIMITIVETYPE : int;

    enum _GLC_CLEAR : uint32_t
    {
        GLC_CLEAR_COLOR = 0x1,
        GLC_CLEAR_ZBUFFER = 0x2,
        GLC_CLEAR_FORCE_DWORD = 0x7FFFFFFFu
    };

    // GLC render-state ids used by ZRenderX86::SetRenderState / GetRenderState (PC).
    // Values 1,2,3,5,7,8 are also consumed by ZRenderWintelD3D::SetRenderState.
    enum _GLC_RENDERSTATE : int
    {
        GLC_RENDERSTATE_ANISOTROPY = 1,
        GLC_RENDERSTATE_ANTIALIAS = 2,
        GLC_RENDERSTATE_DXT = 3,
        GLC_RENDERSTATE_VSYNC = 5,
        GLC_RENDERSTATE_GAMMA = 6,
        GLC_RENDERSTATE_EMBM = 7,
        GLC_RENDERSTATE_TRILINEAR = 8,
        GLC_RENDERSTATE_LOD = 9,
    };
}
