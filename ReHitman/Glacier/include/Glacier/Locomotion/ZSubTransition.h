#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Animation/ZAnimVariationHandle.h>


namespace Glacier::Locomotion
{
    struct ZSubTransition
    {
        ZSubTransition();

        ZAnimVariationHandle m_AnimHandle;
        RE_ADD_PADDING(2);
        float m_StartFramePct;
        float m_EndFramePct;
        bool m_Mirrored;
        RE_ADD_PADDING(3);
        float m_BlendFrames;
    };

    RE_VERIFY_SIZE(ZSubTransition, 0x14);
}
