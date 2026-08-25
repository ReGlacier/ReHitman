#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZUniMemory.h>

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
    // fwds
    struct Manager;
    class ZLNKOBJ;

    struct ZAngelBone
    {
        // members
        ZQuat m_Quat {};
        ZVector3 m_Pos {};
    };
    RE_VERIFY_SIZE(ZAngelBone, 0x1C);

    struct ScaleInfo
    {
        float m_ArmScale{ 1.f };
        float m_HipScale{ 1.f };
    };
    RE_VERIFY_SIZE(ScaleInfo, 0x8);

    union ZAnimHistory
    {
        struct
        {
            uint16_t m_Mask;
            uint16_t m_Id : 15;
            uint16_t m_Mirrored : 1;
        };

        uint32_t m_Raw { 0u };
    };
    RE_VERIFY_SIZE(ZAnimHistory, 0x4);

    struct Model
    {
        // static
        static void ResolveStaticResourceRefs();
        static void GetAimFrames(float &,float &,float &,float,float);

        STATIC_CLASS_VAR(Model, int, m_EyePoseIdOk);
        STATIC_CLASS_VAR(Model, int, m_EyePoseId);
        STATIC_CLASS_VAR(Model, float, g_YCEN);
        STATIC_CLASS_VAR(Model, int, g_UseNewAim);
        STATIC_CLASS_VAR(Model, float, g_EyeLookAtHor);
        STATIC_CLASS_VAR(Model, float, g_EyeLookAtVer);
        STATIC_CLASS_VAR(Model, float, g_AimIkFac);
        STATIC_CLASS_VAR(Model, int, g_AimBasePelvis);
        STATIC_CLASS_VAR(Model, float, _g_AimArmFac);

        // methods
        Model();
        int DynamicSize(ZLNKOBJ* pLnkObj, uint32_t poseIdx, uint32_t id2IndexIdx, uint32_t index2IdIdx, uint32_t id2PosIdx, uint32_t parentIdx, bool stateModel, int boneCount);
        void Init(ZLNKOBJ* pLnkObj, ZBone* pBones, uint32_t poseIdx, uint32_t id2IndexIdx, uint32_t index2IdIdx, uint32_t id2PosIdx, uint32_t parentIdx, bool stateModel, char* buffer, int boneCount);
        int DepackOrder(uint8_t* order);
        void PostAnim(float fUnused);
        void PrepareAnim();
        void PoseRotationAndTranslation();
        void AnimateState(Manager* manager, float fDt);
        void PrintDebugInfo();
        void StateFit(ZAngelBone* pAngelBone);
        void BlendOutPoseWeights();
        void BlendQuats();
        void ResetBones();
        void AnimateQuats(Manager* manager);
        void ModelSpaceBones();
        void LookAt(ZAngelBone* pAngelBone, Manager* pManager, float fDt);
        void EyeLookAt(ZAngelBone* pAngelBone, Manager* pManager, float fDt);
        void Bank(float fDt);

        // members
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
        float              m_Banking[2];
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

    RE_VERIFY_OFFSET(Model, m_Valid, 0xDB); // Verified by ZBoneModifyBase::ForceRagdollDeactivation
}
