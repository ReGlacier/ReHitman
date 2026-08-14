#include <Glacier/Animation/Model.h>


namespace Glacier::Animation
{
    Model::Model()
    {
        for (int i = 3; i != -1; --i)
        {
            m_ActiveAnims[i] = ActiveAnimation();
        }

        for (int i = 6; i != -1; --i)
        {
            m_Targets[i] = IKTarget();
        }

        m_BlendAnim = ActiveAnimation();

        m_Animated = false;
        m_Valid = false;
        m_LastFullBody.m_Raw |= 0x7FFFu;
        m_LastUpperBody.m_Raw |= 0x7FFFu;
        m_BlendBones = nullptr;
        m_BoneIdToIndexLookup = nullptr;
        m_PoseWeights = 0;
        m_PoseIdToPosLookup = 0;
        m_Banking.x = 0.0;
        m_Banking.y = 0.0;
        m_BlendAnim.header = 0;
        m_LastAnim.header = 0;
        m_State = 0;
        m_StateStore = 0;
        m_BoneIdToPosLookup = 0;
        m_State = 0;
        m_StateBlending = 0;
    }
}