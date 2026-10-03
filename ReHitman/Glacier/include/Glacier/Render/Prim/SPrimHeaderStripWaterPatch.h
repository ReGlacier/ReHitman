#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/Render/Prim/SPrimHeaderStrip.h>
#include <cstdint>


namespace Glacier
{
    struct SWaterPatchSineWaveDesc
    {
        ZVector2 m_fWaveDir;
        float m_fWaveSpeed;
        float m_fWaveLength;
        float m_fWaveAmplitude;
    };
    RE_VERIFY_SIZE(SWaterPatchSineWaveDesc, 0x14);

    struct SPrimHeaderStripWaterPatch : SPrimHeaderStrip
    {
        int m_iXSegs;
        int m_iZSegs;
        float m_fXSize;
        float m_fZSize;
        ZVector3 m_vPos;
        uint32_t m_lFlags;
        SWaterPatchSineWaveDesc m_pWavesDesc[4];
        uint32_t m_lGroupID;
        float m_fFresnelMin;
        float m_fFresnelMax;
        float m_fReflectScale;
        float m_fRefractScale;
        uint8_t m_pColor0[4];
        uint8_t m_pColor1[4];
    };
    RE_VERIFY_SIZE(SPrimHeaderStripWaterPatch, 0xC4);
}