#pragma once

#include <cstdint>

namespace Glacier
{
    struct EaxVector
    {
        float x;
        float y;
        float z;
    };

    struct EaxListenerProperties
    {
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
        EaxVector m_vReflectionsPan;
        int32_t m_lReverb;
        float m_fReverbDelay;
        EaxVector m_vReverbPan;
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

    struct EaxBufferProperties
    {
        int32_t m_lDirect;
        int32_t m_lDirectHF;
        int32_t m_lRoom;
        int32_t m_lRoomHF;
        int32_t m_lObstruction;
        float m_fObstructionLFRatio;
        int32_t m_lOcclusion;
        float m_fOcclusionLFRatio;
        float m_fOcclusionRoomRatio;
        float m_fOcclusionDirectRatio;
        int32_t m_lExclusion;
        float m_fExclusionLFRatio;
        int32_t m_lOutsideVolumeHF;
        float m_fDopplerFactor;
        float m_fRolloffFactor;
        float m_fRoomRolloffFactor;
        float m_fAirAbsorptionFactor;
        uint32_t m_lFlags;
    };

    static_assert(sizeof(EaxVector) == 0x0C);
    static_assert(sizeof(EaxListenerProperties) == 0x70);
    static_assert(sizeof(EaxBufferProperties) == 0x48);
}
