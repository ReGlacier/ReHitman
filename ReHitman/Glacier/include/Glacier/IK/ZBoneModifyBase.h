#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZSTL/TIMETYPE.h>
#include <Glacier/ZSTL/ZStackArray.h>
#include <Glacier/IK/SRagdollCollisionInfo.h>
#include <cstdint>


namespace Glacier
{
    // fwds
    class ZIKLNKOBJ;
    class ZLNKOBJ;
    class ZDynamicsExtend;
    class CRagdoll2;
    class ZBoneQuat;
    struct ZBone;

    class ZBoneModifyBase
    {
    public:
        // types
        struct ZAttachGeom
        {
            ZBaseGeom* m_pBaseGeom;
            uint32_t m_lBoneId;
            ZMat3x3 m_mOffset;
            ZVector3 m_vOffset;
        };

        // methods
        bool IsRagdollActive() const;
        bool IsRagdollMoving() const;
        uint8_t DecalLookup() const;
        bool HideBone(ZBaseGeom* pBaseGeom, bool, bool);
        const ZBone* GetBones(const ZLNKOBJ* pLnkObj) const;
        void GetIKBone(const ZBone* pBones, const float* pConvBones, uint32_t lBoneIndex, ZMat3x3& mMat, ZVector3& vPos);
        void GetBoneMatPos(ZMat3x3& mMat, ZVector3& vPos, uint32_t lBoneIdx, const ZLNKOBJ* pLnkObj);
        bool GetIKBoneMatPos(ZMat3x3& mMat, ZVector3& vPos, uint8_t lIndex, const ZLNKOBJ* pLnkObj, ZBone* pBone);
        void PrimChanged(uint32_t lPrim);
        
        const SRagdollCollisionInfo* GetCollisionInfo() const;

        // members
        bool m_bIsPlayer;
        RE_ADD_PADDING(3);
        uint32_t m_lHiddenBoneIds;
        TIMETYPE m_fLastUpdateTime;
        uint32_t m_lLastUpdateFrameCount;
        uint16_t m_lNumActiveBones;
        uint8_t m_lDecalLookup;
        bool m_bPassive;
        ZStackArray<12, ZBoneModifyBase::ZAttachGeom> m_AttachedGeoms;
        float m_fHeadTimePrc;
        ZVector3 m_vHeadTarget;
        ZVector3 m_vRemHeadDirection;
        float m_fAimTimePrc;
        float m_fAimBlendSpeed;
        ZVector3 m_vAimTarget;
        float m_fVisibleDistanceFromCamera;
        ZVector3 m_vSize;
        ZVector3 m_vCenter;
        float m_fGlobalScale;
        CRagdoll2* m_pRagdoll;
        ZDynamicsExtend* m_pDynamicsExt;
        uint16_t m_wBody;
        ZStackArray<4, uint32_t> m_ConnectedPhysics;
    };

    RE_VERIFY_OFFSET(ZBoneModifyBase, m_lDecalLookup, 0x12);
    RE_VERIFY_OFFSET(ZBoneModifyBase, m_pRagdoll, 0x308); // Verified ZBoneModifyBase::IsRagdollMoving


    using ZBoneModifyBase_CB = void(*)(void*, uint32_t, void*);
    using ZBoneModifyBase_GCB = void(*)(ZBone*, uint32_t, ZLNKOBJ*);
    using ZBoneModifyBase_LCB = void(*)(ZBoneQuat*, uint32_t, ZLNKOBJ*);
}
