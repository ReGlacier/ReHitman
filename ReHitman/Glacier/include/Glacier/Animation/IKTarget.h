#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>


namespace Glacier::Animation
{
    struct IKTarget
    {
        ZVector3 m_Pos2;
        float m_Weight2;

        union
        {
            float m_Data[6];

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