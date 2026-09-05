#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Locomotion/ZSubTransition.h>
#include <Glacier/ZSTL/ZMath.h>


namespace Glacier
{
    struct ZAnimTemplatesNames;
}

namespace Glacier::Locomotion
{
    struct ZTransition
    {
        static constexpr int MaxSubTransitions = 1;

        ZTransition();

        ZTransition* Init(ZAnimTemplatesNames& names, int from, int to,
            const char* pszAnimation, float fDirectionX, float fDirectionY);
        ZTransition* Init(ZAnimTemplatesNames& names, int from, int to,
            const char* pszAnimation, const float* pDirection);
        ZTransition* SetBlendIn(float phase);
        ZTransition* SetBlendOut(float phase, float blendFrames);
        ZTransition* SetManualAnimStart(float framePct);
        ZTransition* SetMirrored(bool mirrored);
        ZTransition* SetPreTransition(ZAnimTemplatesNames& names, const char* pszAnimation,
            float startFramePct, float endFramePct, float blendFrames, bool mirrored);
        ZTransition* SetUseGroundFully(bool useGroundFully);

        int addSubTransition(ZAnimTemplatesNames& names, const char* pszAnimation);

        int8_t m_Transition[2];
        RE_ADD_PADDING(2);
        ZVector2 m_Direction;
        float m_BlendInFromPhase;
        float m_BlendOutToPhase;
        float m_QuickStartFramePct;
        int8_t m_SubCount;
        RE_ADD_PADDING(3);
        ZSubTransition* m_Sub[MaxSubTransitions];
        bool m_UseGroundAnimFully;
        RE_ADD_PADDING(3);
    };

    RE_VERIFY_SIZE(ZTransition, 0x24);
}
