#include <Glacier/Locomotion/ZTransition.h>

#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier::Locomotion
{
    ZTransition::ZTransition()
        : m_Transition{-1, -1}
        , m_Direction{0.0f, 1.0f}
        , m_BlendInFromPhase(0.0f)
        , m_BlendOutToPhase(0.0f)
        , m_QuickStartFramePct(0.0f)
        , m_SubCount(0)
        , m_Sub{nullptr}
        , m_UseGroundAnimFully(false)
    {
    }

    int ZTransition::addSubTransition(ZAnimTemplatesNames& names, const char* pszAnimation)
    {
        ZASSERT(m_SubCount < MaxSubTransitions);
        const int index = m_SubCount++;
        m_Sub[index] = new ZSubTransition();
        names.FindAnimVariationHandle(m_Sub[index]->m_AnimHandle, pszAnimation);
        return index;
    }

    ZTransition* ZTransition::Init(ZAnimTemplatesNames& names, int from, int to,
        const char* pszAnimation, float fDirectionX, float fDirectionY)
    {
        m_Transition[0] = static_cast<int8_t>(from);
        m_Transition[1] = static_cast<int8_t>(to);
        addSubTransition(names, pszAnimation);
        m_Direction = {fDirectionX, fDirectionY};
        const float length = std::sqrt(fDirectionX * fDirectionX + fDirectionY * fDirectionY);
        if (length != 0.0f)
        {
            m_Direction.x /= length;
            m_Direction.y /= length;
        }
        m_BlendInFromPhase = 0.0f;
        m_BlendOutToPhase = 0.0f;
        m_QuickStartFramePct = 0.0f;
        return this;
    }

    ZTransition* ZTransition::Init(ZAnimTemplatesNames& names, int from, int to,
        const char* pszAnimation, const float* pDirection)
    {
        return Init(names, from, to, pszAnimation, pDirection[0], pDirection[1]);
    }

    ZTransition* ZTransition::SetBlendIn(float phase)
    {
        m_BlendInFromPhase = phase;
        return this;
    }

    ZTransition* ZTransition::SetBlendOut(float phase, float blendFrames)
    {
        if (m_SubCount > 0)
        {
            m_Sub[0]->m_EndFramePct = phase;
            m_BlendOutToPhase = blendFrames;
        }
        return this;
    }

    ZTransition* ZTransition::SetManualAnimStart(float framePct)
    {
        m_QuickStartFramePct = framePct;
        return this;
    }

    ZTransition* ZTransition::SetMirrored(bool mirrored)
    {
        if (m_SubCount > 0)
            m_Sub[0]->m_Mirrored = mirrored;
        return this;
    }

    ZTransition* ZTransition::SetPreTransition(ZAnimTemplatesNames& names,
        const char* pszAnimation, float startFramePct, float endFramePct,
        float blendFrames, bool mirrored)
    {
        const int index = addSubTransition(names, pszAnimation);
        m_Sub[index]->m_StartFramePct = startFramePct;
        m_Sub[index]->m_EndFramePct = endFramePct;
        m_Sub[index]->m_BlendFrames = blendFrames;
        m_Sub[index]->m_Mirrored = mirrored;
        return this;
    }

    ZTransition* ZTransition::SetUseGroundFully(bool useGroundFully)
    {
        m_UseGroundAnimFully = useGroundFully;
        return this;
    }
}
