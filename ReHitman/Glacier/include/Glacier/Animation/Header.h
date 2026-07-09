#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier::Animation
{
    struct Header
    {
        int16_t m_States;
        int16_t m_OrgStartFrame;
        int16_t m_Frames;
        int16_t m_RealFPS;
        int32_t m_Mask;
        int32_t m_Size;
        int32_t m_StateOffset;
        int32_t m_QuatOffset;
        int32_t m_GroundOffset;
        int32_t m_MetaDataOffset;
        int32_t m_PoseDataOffset;
        float m_BlendFrames;
        float m_CycleDist[3];
        int32_t m_OldControl;
        int32_t m_SoundIndex;
        char* m_Name;
    };
    // Need verify later because on PS2 it's 0x40, in Mini Ninjas it's 0x50, but looks like it's 0x40
    RE_VERIFY_SIZE(Header, 0x40);
}