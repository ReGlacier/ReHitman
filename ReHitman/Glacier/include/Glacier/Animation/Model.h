#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>

#include <Glacier/Animation/ZPoseCollection.h>
#include <Glacier/Animation/ActiveAnimation.h>
#include <Glacier/Animation/ZStateBlending.h>
#include <Glacier/Animation/ZHumanState.h>
#include <Glacier/Animation/ZBlendBone.h>
#include <Glacier/Animation/IKTarget.h>
#include <Glacier/Animation/ZBone.h>

#include <cstdint>


namespace Glacier::Animation
{
    struct ZAngelBone 
    {
        ZQuat m_Quat;
        ZVector3 m_Pos;
    };
    RE_VERIFY_SIZE(ZAngelBone, 0x1C);

    struct ScaleInfo 
    {
        float m_ArmScale;
        float m_HipScale;
    };
    RE_VERIFY_SIZE(ScaleInfo, 0x8);

    union ZAnimHistory 
    {
        uint16_t m_Mask;
        struct 
        {
            uint16_t m_Id : 15;
            uint16_t m_Mirrored : 1;
        };
        uint32_t raw_padding;
    };
    RE_VERIFY_SIZE(ZAnimHistory, 0x4);

    struct Model
    {
        int16_t*           m_PoseIdToPosLookup;
        ZAngelBone*        m_AngelPose;
        BoneID*            m_BoneIndexToIdLookup;
        BoneIndex*         m_BoneIdToIndexLookup;
        BoneIndex*         m_Parent;
        ScaleInfo          m_ScaleInfo;
        ZPoseCollection    m_Poses;
        int16_t*           m_BoneIdToPosLookup;
        ActiveAnimation    m_ActiveAnims[4];
        uint8_t            m_DepackOrder[4];
        uint8_t            m_OrderSize;
        bool               m_Animated;
        bool               m_FaceAnimated;
        bool               m_Valid;
        ZAnimHistory       m_LastFullBody;
        ZAnimHistory       m_LastUpperBody;
        IKTarget           m_Targets[7];
        int                m_BoneCount;
        ActiveAnimation    m_BlendAnim;
        ActiveAnimation    m_LastAnim;
        ZVector2           m_Banking;
        ZAngelBone         m_PelvisPlacement;
        float              m_PelvisPlacementWeight;
        ZVector3           m_AimDir;
        float*             m_PoseWeights;
        ZStateBlending*    m_StateBlending;
        ZBone*             m_Bones;
        ZHumanState*       m_State;
        char*              m_StateStore;
        ZBlendBone*        m_BlendBones;
    };
    RE_VERIFY_SIZE(Model, 0x2A4); // Verified
}