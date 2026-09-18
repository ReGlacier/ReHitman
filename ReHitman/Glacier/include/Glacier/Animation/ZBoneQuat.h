#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    class ZBoneQuat
    {
    public:
        // methods
        // ZBoneQuat();

        // members
        float m_vQuat[4];
        float m_Align[3];
        union
        {
            float   m_Blend;
            int32_t m_lBlend;
        };
        int32_t m_MagicNr;
        float m_vPosition[3];
    };
    RE_VERIFY_SIZE(ZBoneQuat, 0x30);
}
