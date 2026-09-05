#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>


namespace Glacier::Animation
{
    struct IKTarget
    {
        // methods
        IKTarget()
        {
            m_Pos2.Reset();
            memset(&m_Data[0], 0, sizeof(m_Data));
            m_Weight2 = 0.0f;
        }

        IKTarget(const IKTarget& copy)
        {
            m_Pos2 = copy.m_Pos2;
            memcpy(&m_Data[0], &copy.m_Data[0], sizeof(m_Data));
            m_Weight2 = copy.m_Weight2;
        }

        IKTarget& operator=(const IKTarget& copy)
        {
            m_Pos2 = copy.m_Pos2;
            memcpy(&m_Data[0], &copy.m_Data[0], sizeof(m_Data));
            m_Weight2 = copy.m_Weight2;
            return *this;
        }

        // members
        ZVector3 m_Pos2{};
        float m_Weight2{1.f};

        union
        {
            float m_Data[6] { 0.f };

            struct 
            {
                ZVector2 m_Blend;
                int m_Mode;
                float m_Speed;
            } m_LookAt2;

            struct 
            {
                ZVector2 m_Blend;
                int8_t m_Mode;
                bool b_Inplace;
                float m_fAimTrackSpeed;    
            } m_Aim;

            struct 
            {
                ZQuat m_Quat;
                int16_t m_BoneIndex;
                char m_Mode;
                char m_Flag;
            } m_Target;

            struct {
                ZQuat m_Quat;
            } m_SpineLock;
        };
    };
    RE_VERIFY_SIZE(IKTarget, 0x28);
}