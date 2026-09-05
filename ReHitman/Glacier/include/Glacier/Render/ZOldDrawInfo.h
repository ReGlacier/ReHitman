#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    /**
     * DronCode: This class reversed from PS2 build because XBox build is not compatible with PC code
     *           Also, original PS2 code not compatible too. Padding at 0x844 created manually
     */
    struct ZOldDrawInfo
    {
        uint32_t m_lNrTriangles;
        uint32_t m_lNrDot3Triangles;
        uint32_t m_lNrTrianglesBones;
        uint32_t m_lNrPrimitives;
        uint32_t m_lNrSprites;
        uint32_t m_lNrSubList;
        uint32_t m_lNrShadowCasters;
        uint32_t m_lNrShadowRecievers;
        uint32_t m_lNrShadowCastersDrawn;
        uint32_t m_lNrShadowRecieversDraw;
        uint32_t m_lNrWeatherBoxes;
        uint32_t m_lNrSplashTri;
        uint32_t m_lNrVisiSplashTri;
        uint32_t m_lNrDrawCalls;
        uint32_t m_lNrBoneChunks;
        uint32_t m_lNrBonePrims;
        uint32_t m_lNrBoneVertices;
        uint8_t m_UsedBoneTextures[2048];
        uint32_t m_Unknown844[2];
        ZVector3 m_vCameraPosition;
    };
    RE_VERIFY_OFFSET(ZOldDrawInfo, m_vCameraPosition, 0x84C); // Approved by ZRenderViewBase::GetDrawInfo
}