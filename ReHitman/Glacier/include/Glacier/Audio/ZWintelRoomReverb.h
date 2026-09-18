#pragma once

#include <Glacier/ReGlacier.h>

#include <cstdint>

namespace Glacier
{
    struct ZWintelRoomReverb
    {
        RE_ADD_PADDING(0x18);
        uint32_t m_lEnvironment;
        float m_fEnvironmentSize;
        float m_fEnvironmentDiffusion;
        int32_t m_lRoom;
        int32_t m_lRoomHF;
        int32_t m_lRoomLF;
        float m_fDecayTime;
        float m_fDecayHFRatio;
        float m_fDecayLFRatio;
        int32_t m_lReflections;
        float m_fReflectionsDelay;
        int32_t m_lReverb;
        float m_fReverbDelay;
        float m_fEchoTime;
        float m_fEchoDepth;
        float m_fModulationTime;
        float m_fModulationDepth;
        float m_fAirAbsorptionHF;
        float m_fHFReference;
        float m_fLFReference;
        float m_fRoomRolloffFactor;
        uint32_t m_lFlags;
    };

    RE_VERIFY_OFFSET(ZWintelRoomReverb, m_lEnvironment, 0x18);
    RE_VERIFY_SIZE(ZWintelRoomReverb, 0x70);
}
