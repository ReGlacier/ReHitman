#pragma once

#include "Glacier/IK/ZLNKOBJ.h"
#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZSTL/TIMETYPE.h>
#include <Glacier/IK/SRagdollCollisionInfo.h>
#include <Glacier/Physics/Fwd.h>
#include <cstdint>


namespace Glacier
{
    enum eBoneID : uint8_t
    {
        NA = 0,
        GROUND = 1,
        PELVIS = 2,
        SPINE = 3,
        SPINE_1 = 4,
        SPINE_2 = 5,
        NECK = 6,
        HEAD = 7,
        PONYTAIL = 8,
        LEFT_THIGH = 9,
        LEFT_CALF = 10,
        LEFT_FOOT = 11,
        LEFT_TOE = 12,
        RIGHT_THIGH = 13,
        RIGHT_CALF = 14,
        RIGHT_FOOT = 15,
        RIGHT_TOE = 16,
        LEFT_CLAVICLE = 17,
        LEFT_UPPER_ARM = 18,
        LEFT_FOREARM = 19,
        LEFT_HAND = 20,
        LEFT_FINGER_0 = 21,
        RIGHT_CLAVICLE = 22,
        RIGHT_UPPER_ARM = 23,
        RIGHT_FOREARM = 24,
        RIGHT_HAND = 25,
        RIGHT_FINGER_0 = 26,
        RIGHT_FINGER_0_1 = 27,
        RIGHT_FINGER_0_2 = 28,
        RIGHT_FINGER_0_3 = 29,
        RIGHT_FINGER_0_4 = 30,
        RIGHT_HAND_ATTACHER = 31,
        LEFT_HAND_ATTACHER = 32,
        FREE_ATTACHER = 33,
    };

    class CRagdoll2
    {
    public:
        // constants
        static constexpr int MAX_PARTICLE_PROPS_NR = 16;

        // types
        struct ParticleProperty
        {
            // methods
            ParticleProperty();

            // members
            int timeout;
            bool fixed;
            RE_ADD_PADDING(3);
            float fixed_pos[3];
        };

        // vtbl
        virtual ~CRagdoll2();

        // methods
        CRagdoll2(bool bRagdoll);
        void LoadSave(ISerializerStream& stream, bool bSaving);
        bool Setup(ZLNKOBJ* pLnkObj);
        void Hit(const float*, const float*, float);
        void InitIndices();
        void CreateParticles();
        uint16_t GetBoneIndex(eBoneID eBone) const;
        void PrimChanged(uint32_t lPrim);
        bool IsRagdoll() const;
        bool IsMoving() const;
        bool IsActive() const;
        uint16_t GetBoneIndex(eBoneID eBone);
        void EnableTimeOut(bool bEnable);
        void Activate(const ZBone* pBones, bool bActivate);
        void Deactivate();
        void SetDamping(float fDamping);
        void GetLocalPelvis(ZVector3& vDir, ZVector3& vPos);
        void ComputeParticlePositions(int lBoneCount, const ZBone* pBones);
        void ComputeParticlePositionsSub(const ZBone* pBones, bool bCalcVelocity);
        bool GetLinkMatPos_global(uint16_t lBoneIndex, ZMat3x3& mMat, ZVector3& vPos, ZVector3* vCenter, ZVector3* vSize, const ZBone* pBones);
        void End();
        bool Move(ZMat3x3& mMat, ZVector3& vPos, float fDt);
        void CalcGroundOffset(ZMat3x3& mMat, ZVector3& vPos);
        void HandleCalcMatsMsg(ZBone* pBone, bool bConvertToQuat, int nNumBones);
        void SetLinkMatPosByIx(int lIndex, const ZMat3x3& mMat, const ZVector3& vPos, ZBone* pBones);
        void LinkMats(int lBoneIndex, ZBone* pBones);
        void CopyBone(int lDstBoneIdx, int lSrcBoneIdx, ZBone* pBones);
        const ZLNKOBJ* LnkObj() const;
        void Init(ZLNKOBJ* pLnkObj, const ZBone* pBones, int lBoneCount);

        // members
        ParticleProperty m_aParticleProperty[MAX_PARTICLE_PROPS_NR];
        uint16_t m_nFixedPars;
        RE_ADD_PADDING(2);
        ConstrainedParticleSystem* m_pParticles;
        SRagdollCollisionInfo m_ColiInfo;
        uint16_t m_nPelvisIx;
        uint16_t m_nSpineIx;
        uint16_t m_nSpine1Ix;
        uint16_t m_nNeckIx;
        uint16_t m_nHeadIx;
        uint16_t m_nLClavicleIx;
        uint16_t m_nLUpperArmIx;
        uint16_t m_nLLowerArmIx;
        uint16_t m_nLHandIx;
        uint16_t m_nRClavicleIx;
        uint16_t m_nRUpperArmIx;
        uint16_t m_nRLowerArmIx;
        uint16_t m_nRHandIx;
        uint16_t m_nLThighIx;
        uint16_t m_nLCalfIx;
        uint16_t m_nLFootIx;
        uint16_t m_nRThighIx;
        uint16_t m_nRCalfIx;
        uint16_t m_nRFootIx;
        uint16_t m_nLForeTwistIx;
        uint16_t m_nRForeTwistIx;
        RE_ADD_PADDING(2);
        ZVector3 m_vGroundToPelvis;
        ZVector3 m_vOldPos;
        float m_fLastDeltaTime;
        uint16_t m_lNumBones;
        bool m_bDragdoll;
        bool m_bActive;
        bool m_bRagdoll;
        RE_ADD_PADDING(3);
        int32_t m_nTimeOut;
        float m_fThreshold;
        float m_fWeightedSpeed;
        int32_t m_nNoMove;
        TIMETYPE m_fStartTime;
        const ZBone* m_pJesusLocalBones;
        const ZLNKOBJ* m_pLnkObj;
    };

    RE_VERIFY_SIZE(CRagdoll2, 0x23C); // Verified PC alloc
    RE_VERIFY_OFFSET(CRagdoll2, m_ColiInfo, 0x14C); // Verified by ctor
    RE_VERIFY_OFFSET(CRagdoll2, m_nPelvisIx, 0x1D0); // Verified by CRagdoll2::InitIndices
    RE_VERIFY_OFFSET(CRagdoll2, m_pParticles, 0x148); // Verified by dtor
    RE_VERIFY_OFFSET(CRagdoll2, m_pLnkObj, 0x238); // Verified by CRagdoll2::InitIndices
}
