#pragma once

#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZCAMERA.h>
#include <Glacier/ZRenderWintelD3D.h>

namespace Glacier {
    class ZRenderDrawD3D;
    class ZRenderEntryCameraD3D;
    class ZRenderEntry;

    class ZDrawBuffer
    {
    public:
        // data
        int32_t m_unk4; //0x0004
        ZRenderEntry** m_pVSpritesArray; //0x0008
        int32_t m_iViewportWidth; //0x000C
        int32_t m_iViewportHeight; //0x0010
        char pad_0014[4]; //0x0014
        ZRenderEntry** m_pVGeomsArray; //0x0018
        ZRenderDrawD3D* m_pRenderDraw; //0x001C
        char pad_0020[4]; //0x0020
        ZRenderWintelD3D* m_pRenderWintel; //0x0024
        char pad_0028[16]; //0x0028
        ZCAMERA* m_pCamera; //0x0038
        ZRenderEntryCameraD3D* m_pRenderEntryCamera; //0x003C
        char pad_0040[4]; //0x0040
        ZVector4* m_pV2Viewport; //0x0044
        // vftable

        virtual int32_t GetSizeX(); // return [0xC]
        virtual int32_t GetSizeY(); // return [0x10]
        virtual double GetPixelAspectXY();
        // possible DrawBonesArray
        // possible DrawSpritesArray

    }; //Size: 0x02C0

    using ZDrawBufferSimple = ZDrawBuffer; // Alias for HBM
}