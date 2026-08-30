#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/Draw/IDraw.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Render/Prim/ZBoneConstraintsHeader.h>
#include <Glacier/Render/Prim/ZBoneConstraintLookAt.h>
#include <Glacier/IK/ZBoneModifyBase.h>
#include <Glacier/Physics/ZRagdollContainer.h>
#include <Glacier/Physics/ZDynamicsExtend.h>
#include <Glacier/Physics/CRagdoll2.h>
#include <Glacier/Animation/Model.h>
#include <Glacier/Animation/ZBone.h>
#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/Geom/ZGeomBuffer.h>
#include <cstring>



namespace Glacier
{
    ZBoneModifyBase::~ZBoneModifyBase()
    {
        if (m_pRagdoll)
        {
            auto* pRagdollContainer = g_pRenderDll->GetRagdollContainer();
            if (pRagdollContainer)
            {
                pRagdollContainer->DeactivateRagdoll(m_pRagdoll);
                m_pRagdoll = nullptr;
            }
        }
    }

    ZBoneModifyBase::ZBoneModifyBase(uint32_t lNrBones)
    {
        m_lDecalLookup = lDecalLookup++;
        m_fLastUpdateTime = {};
        m_AttachedGeoms = {};
        m_ConnectedPhysics = {};
        m_fGlobalScale = 1.0f;
        m_lHiddenBoneIds = 0;
        m_fLastUpdateTime.secs = -0x400u;
        m_lLastUpdateFrameCount = 0;
        m_fAimTimePrc = 0.f;
        m_fAimBlendSpeed = 0.05f;
        m_bIsPlayer = false;
        m_lNumActiveBones = lNrBones;
        ZASSERT(lNrBones > 0);
        m_bPassive = false;
        m_fHeadTimePrc = 0.0f;
        m_fVisibleDistanceFromCamera = 0.0f;
        m_vRemHeadDirection = { 0.0f };
        m_vSize = { 1.0f };
        m_vCenter = { 0.0f };
        m_pRagdoll = nullptr;
        m_pDynamicsExt = nullptr;
    }

    bool ZBoneModifyBase::IsRagdollActive() const
    {
        return m_pRagdoll && m_pRagdoll->IsActive();
    }

    bool ZBoneModifyBase::IsRagdollMoving() const
    {
        return m_pRagdoll && m_pRagdoll->IsMoving();
    }

    uint8_t ZBoneModifyBase::DecalLookup() const
    {
        return m_lDecalLookup;
    }

    bool ZBoneModifyBase::HideBone(ZBaseGeom* pBaseGeom, uint8_t lBoneIndex, bool bHide)
    {
        uint32_t lBoneBit = 1 << lBoneIndex;
        uint32_t lCurrentMask = m_lHiddenBoneIds;

        if (bHide)
        {
            if ((lCurrentMask & lBoneBit) == 0)
            {
                m_lHiddenBoneIds = lCurrentMask | lBoneBit;
                return 0;
            }
        }
        else
        {
            if ((lCurrentMask & lBoneBit) == 0)
            {
                return 0;
            }

            m_lHiddenBoneIds = lCurrentMask & ~lBoneBit;
        }

        return 1;
    }

    void ZBoneModifyBase::GetIKBone(const ZBone* pBones, const float* pConvBones, uint32_t lBoneIndex, Matrix3x3& mMat, Vector3& vPos) const
    {
        const auto& bone = pBones[lBoneIndex];
        const auto& mConvMat = reinterpret_cast<const Matrix3x3*>(pConvBones)[lBoneIndex];

        vPos = bone._Pos;

        Matrix3x3 mTransposed = bone._Mat.TransposedAntidiagonal();
        mTransposed *= mConvMat;
        mMat = mTransposed.TransposedAntidiagonal();
    }

    const ZBone* ZBoneModifyBase::GetBones(const ZLNKOBJ* pLnkObj) const
    {
        const ZBone* pBones = IDraw::Instance()->GetBaseGeomBones(pLnkObj->BaseGeom());
        if (!pBones)
        {
            // it's ok due ZBone is POD type contains only 'float' entries.
            return reinterpret_cast<const ZBone*>(ZPrimControlBase::Instance()->GetGlobalPrimBones(pLnkObj->Prim()));
        }

        ZASSERT(pLnkObj->Model()->m_Valid);
        return pBones;
    }

    const SRagdollCollisionInfo* ZBoneModifyBase::GetCollisionInfo() const
    {
        return m_pRagdoll ? &m_pRagdoll->m_ColiInfo : nullptr;
    }

    void ZBoneModifyBase::GetBoneMatPos(ZMat3x3& mMat, ZVector3& vPos, uint32_t lBoneIdx, const ZLNKOBJ* pLnkObj, ZBone* pBone) const
    {
        const auto* pLUT = ZPrimControlBase::Instance()->GetBoneIdToIndexLookup(pLnkObj->Prim());
        if (!GetIKBoneMatPos(mMat, vPos, pLUT[lBoneIdx], pLnkObj, pBone))
        {
            mMat.Reset();
            vPos.Reset();
        }
    }

    bool ZBoneModifyBase::GetIKBoneMatPos(ZMat3x3& mMat, ZVector3& vPos, uint8_t lIndex, const ZLNKOBJ* pLnkObj, ZBone* pBone) const
    {
        if (lIndex > m_lNumActiveBones)
        {
            return false;
        }

        const auto* pConvBones = ZPrimControlBase::Instance()->GetConvBones(pLnkObj->Prim());

        if (pBone)
        {
            GetIKBone(pBone, pConvBones, lIndex, mMat, vPos);
            return true;
        }

        auto* pBaseBones = IDraw::Instance()->GetBaseGeomBones(pLnkObj->BaseGeom());
        if (pBaseBones)
        {
            GetIKBone(pBaseBones, pConvBones, lIndex, mMat, vPos);
            return true;
        }

        if (!m_bPassive)
        {
            pBaseBones = reinterpret_cast<const ZBone*>(ZPrimControlBase::Instance()->GetGlobalPrimBones(pLnkObj->Prim()));
            GetIKBone(pBaseBones, pConvBones, lIndex, mMat, vPos);
            return true;
        }

        // Fallback
        ZBone aBones[MAXNRBONESPERPRIM];
        pLnkObj->GetDefaultBones(aBones, 0);
        GetIKBone(aBones, pConvBones, lIndex, mMat, vPos);

        return true;
    }

    bool ZBoneModifyBase::AttachBaseGeomToBone(const ZBaseGeom* pBaseGeom, uint32_t lBoneId, const float* pMat, const float* pPos)
    {
        const ZREF rBaseGeom = pBaseGeom ? ZGeomBuffer::Instance().GeomPtrToRef(pBaseGeom) : 0;
        if (m_AttachedGeoms.Count() >= m_AttachedGeoms.TotalNrEntries())
            return false;

        for (uint32_t i = 0; i < m_AttachedGeoms.Count(); ++i)
        {
            if (m_AttachedGeoms.Get(i)->m_rBaseGeom == rBaseGeom)
                return false;
        }

        ZAttachGeom attached {};
        attached.m_rBaseGeom = rBaseGeom;
        attached.m_lBoneId = lBoneId;
        if (pMat)
            std::memcpy(attached.m_mOffset.data, pMat, sizeof(attached.m_mOffset));
        else
            attached.m_mOffset.Reset();
        if (pPos)
            std::memcpy(&attached.m_vOffset, pPos, sizeof(attached.m_vOffset));
        else
            attached.m_vOffset.Reset();

        m_AttachedGeoms.Add(&attached);
        IDraw::Instance()->EnableOwnerDraw(pBaseGeom);
        return true;
    }

    void ZBoneModifyBase::DetachBaseGeomFromBone(const ZBaseGeom* pBaseGeom, uint32_t lBoneId)
    {
        const ZREF rBaseGeom = pBaseGeom ? ZGeomBuffer::Instance().GeomPtrToRef(pBaseGeom) : 0;
        for (uint32_t i = 0; i < m_AttachedGeoms.Count(); ++i)
        {
            const auto* pAttached = m_AttachedGeoms.Get(i);
            if (pAttached->m_rBaseGeom == rBaseGeom && pAttached->m_lBoneId == lBoneId)
            {
                IDraw::Instance()->DisableOwnerDraw(pBaseGeom);
                m_AttachedGeoms.Remove(i);
                return;
            }
        }
    }

    uint32_t ZBoneModifyBase::GetAttachedBaseGeomBoneId(const ZBaseGeom* pBaseGeom) const
    {
        const ZREF rBaseGeom = pBaseGeom ? ZGeomBuffer::Instance().GeomPtrToRef(pBaseGeom) : 0;
        for (uint32_t i = 0; i < m_AttachedGeoms.Count(); ++i)
        {
            const auto* pAttached = m_AttachedGeoms.Get(i);
            if (pAttached->m_rBaseGeom == rBaseGeom)
                return pAttached->m_lBoneId;
        }
        return 0;
    }

    bool ZBoneModifyBase::FindAttachedGeomMatPos(ZMat3x3& mMat, ZVector3& vPos, const ZBaseGeom* pBaseGeom, const ZLNKOBJ* pLnkObj) const
    {
        const ZREF rBaseGeom = pBaseGeom ? ZGeomBuffer::Instance().GeomPtrToRef(pBaseGeom) : 0;
        const ZAttachGeom* pAttached = nullptr;
        for (uint32_t i = 0; i < m_AttachedGeoms.Count(); ++i)
        {
            if (m_AttachedGeoms.Get(i)->m_rBaseGeom == rBaseGeom)
            {
                pAttached = m_AttachedGeoms.Get(i);
                break;
            }
        }
        if (!pAttached)
            return false;

        GetBoneMatPos(mMat, vPos, pAttached->m_lBoneId, pLnkObj);
        ZVector3 offset;
        vmmul(offset, pAttached->m_vOffset, mMat);
        vPos += offset;
        ZMat3x3 result;
        mmmul(result, pAttached->m_mOffset, mMat);
        mMat = result;
        return true;
    }

    void ZBoneModifyBase::PrimChanged(uint32_t lPrim)
    {
        m_lNumActiveBones = ZPrimControlBase::Instance()->GetNrBones(lPrim);
        if (m_pRagdoll)
        {
            m_pRagdoll->PrimChanged(lPrim);
        }
    }

    void ZBoneModifyBase::ForceRagdollDeactivation(ZLNKOBJ* pLnkObj)
    {
        ZBone aBones[MAXNRBONESPERPRIM];

        ZASSERT(pLnkObj);
        if (!pLnkObj)
            return;

        ZASSERT(m_lNumActiveBones <= MAXNRBONESPERPRIM);

        const auto* pBones = IDraw::Instance()->GetBaseGeomBones(pLnkObj->BaseGeom());
        if (pBones)
        {
            ZASSERT(pLnkObj->Model()->m_Valid);
        }
        else
        {
            pBones = reinterpret_cast<const ZBone*>(ZPrimControlBase::Instance()->GetGlobalPrimBones(pLnkObj->Prim()));
        }

        if (m_lNumActiveBones)
        {
            for (int i = 0; i < m_lNumActiveBones; ++i)
            {
                aBones[i] = pBones[i];
            }
        }

        m_pRagdoll->HandleCalcMatsMsg(aBones, false, m_lNumActiveBones);

        const auto* pBoneDefs = pLnkObj->GetBoneDefinitions();
        pLnkObj->SetDefaultBones(aBones, pBoneDefs);

        g_pRenderDll->GetRagdollContainer()->DeactivateRagdoll(m_pRagdoll);
        m_pRagdoll = nullptr;
        m_bPassive = true;
        pLnkObj->OnMoved();
    }

    bool ZBoneModifyBase::Update(ZLNKOBJ* pLnkObj, ZMat3x3& mMat, ZVector3& vPos)
    {
        if (m_lLastUpdateFrameCount == g_pSysInterface->m_lFrameCount)
        {
            return false;
        }

        const float fDt = g_pSysInterface->FrameTime - m_fLastUpdateTime;
        m_lLastUpdateFrameCount = g_pSysInterface->m_lFrameCount;
        m_fLastUpdateTime = g_pSysInterface->FrameTime;

        if (m_pRagdoll)
        {
            if (m_pRagdoll->IsActive())
            {
                m_bPassive = false;
            }
        }
        else if (!m_pDynamicsExt)
        {
            return false;
        }

        if (m_pDynamicsExt && m_pDynamicsExt->Awake(m_wBody))
        {
            m_bPassive = false;
        }

        if (!m_bPassive)
        {
            vPos.Reset();
            mMat.Reset();

            if (m_pDynamicsExt)
            {
                m_pDynamicsExt->Move(m_wBody, pLnkObj, fDt);

                if (!m_pDynamicsExt->Awake(m_wBody))
                {
                    const auto* pBoneDefs = pLnkObj->GetBoneDefinitions();
                    const auto* pBones = pLnkObj->GetBones();

                    pLnkObj->SetDefaultBones(pBones, pBoneDefs);
                    pLnkObj->OnMoved();
                    m_bPassive = true;
                }

                return true;
            }

            if (!m_pRagdoll || !m_pRagdoll->IsActive() || m_pRagdoll->Move(mMat, vPos, fDt))
            {
                return true;
            }

            ForceRagdollDeactivation(pLnkObj);
        }

        return false;
    }

    namespace
    {
        void GetTargetPosition(ZVector3& vTarget, const ZBone* pBones, const ZBoneConstraintLookAt* pBoneConstraintLookAt)
        {
            vTarget.Reset();

            if (!pBoneConstraintLookAt->m_lNrTargets)
                return;

            ZASSERT(pBoneConstraintLookAt->m_lNrTargets <= 2); // otherwise out of bounds

            float fWeightsSum = 0.0f;
            ZVector3 v1;

            for (int i = 0; i < pBoneConstraintLookAt->m_lNrTargets; ++i)
            {
                float fWeight = pBoneConstraintLookAt->m_lBoneTargetsWeights[i];

                vmmul(v1, pBoneConstraintLookAt->m_TargetPos[i], pBones[pBoneConstraintLookAt->m_TargetParentIdx[i]]._Mat);
                vadd(v1, pBones[pBoneConstraintLookAt->m_TargetParentIdx[i]]._Pos);
                vaddscalar(vTarget, vTarget, v1, fWeight);

                fWeightsSum += fWeight;
            }

            if (fWeightsSum >= 0.0f)
            {
                const float fInvWeightSum = 1.0f / fWeightsSum;
                vscalar(vTarget, fInvWeightSum);
            }
        }

        void ProjectionOnPerpPlane(ZVector3& vRes, const ZVector3& vProjectionOf, const ZVector3& vOnPlatePerpTo, bool bFlip)
        {
            ZVector3 vCross;
            vcross(vCross, vOnPlatePerpTo, vProjectionOf);

            // Are they parallel?
            if (vlen2(vCross) < 1.4901161e-8f)
            {
                // Find index of lower component of vectors
                int lMinIndex = 0;
                if (vOnPlatePerpTo.x > vOnPlatePerpTo.y)
                    lMinIndex = 1;

                const float* components = &vOnPlatePerpTo.x;
                if (components[lMinIndex] > vOnPlatePerpTo.z)
                    lMinIndex = 2;

                // Make axis vector
                vCross.Reset();
                static_cast<float*>(vCross)[lMinIndex] = 1.0f;

                // Make ortho-vector to vOnPlatePerpTo
                ZVector3 vOrtho {};
                vcross(vOrtho, vCross, vOnPlatePerpTo);
                vcross(vCross, vOnPlatePerpTo, vOrtho);
                vnorm(vCross);
            }

            vcross(vRes, vCross, vOnPlatePerpTo);
            vnorm(vRes);
            if (bFlip)
                vneg(vRes);
        }

        void CalculateLookAtMatrix(ZMat3x3& m0, const ZBone* pBones, const ZBoneConstraintLookAt* pBoneConstraintLookAt)
        {
            const ZVector3& vSource = pBones[pBoneConstraintLookAt->m_lBoneIndex]._Pos;

            ZVector3 vTarget;
            GetTargetPosition(vTarget, pBones, pBoneConstraintLookAt);

            ZVector3 vLookAt;
            vsub(vLookAt, vTarget, vSource);
            vnorm(vLookAt);
            if (pBoneConstraintLookAt->m_lLookAtFlip)
                vneg(vLookAt);

            const auto& upNode = pBones[pBoneConstraintLookAt->m_UpNodeParentIdx];

            ZVector3 vUpNodePosition;
            vmmul(vUpNodePosition, pBoneConstraintLookAt->m_UpPos, upNode._Mat);
            vadd(vUpNodePosition, upNode._Pos);

            ZVector3 vSourceToUpNode;
            vsub(vSourceToUpNode, vUpNodePosition, vSource);
            if (vSourceToUpNode.x == 0.0f && vSourceToUpNode.y == 0.0f && vSourceToUpNode.z == 0.0f)
                vSourceToUpNode = { 1.0f, 0.0f, 0.0f };
            vnorm(vSourceToUpNode);

            ZVector3 vLocalUpPosition;
            if (pBoneConstraintLookAt->m_UpnodeControl)
                vmmul(vLocalUpPosition, pBoneConstraintLookAt->m_UpPos, upNode._Mat);

            auto projectUp = [&](ZVector3& vResult, const ZVector3& vAxis, bool bLocalFlip, bool bUpNodeFlip)
            {
                if (pBoneConstraintLookAt->m_UpnodeControl == 1)
                    ProjectionOnPerpPlane(vResult, vLocalUpPosition, vAxis, bLocalFlip);
                else
                    ProjectionOnPerpPlane(vResult, vSourceToUpNode, vAxis, bUpNodeFlip);
            };

            if (pBoneConstraintLookAt->m_lLookAtAxis == 0)
            {
                m0.ZAxis() = vLookAt;

                if (pBoneConstraintLookAt->m_lUpBoneAlignmentAxis == 6)
                {
                    projectUp(m0.XAxis(), m0.ZAxis(), false, pBoneConstraintLookAt->m_lUpFlip != 0);
                    vneg(m0.ZAxis());
                    vcross(m0.YAxis(), m0.ZAxis(), m0.XAxis());
                    vnorm(m0.YAxis());
                }
                else if (pBoneConstraintLookAt->m_lUpBoneAlignmentAxis == 3)
                {
                    projectUp(m0.YAxis(), m0.ZAxis(), false, pBoneConstraintLookAt->m_lUpFlip != 0);
                    vcross(m0.XAxis(), m0.YAxis(), m0.ZAxis());
                    vnorm(m0.XAxis());
                    vneg(m0.ZAxis());
                    vneg(m0.XAxis());
                }
            }
            else if (pBoneConstraintLookAt->m_lLookAtAxis == 3)
            {
                m0.YAxis() = vLookAt;

                if (pBoneConstraintLookAt->m_lUpBoneAlignmentAxis == 6)
                {
                    projectUp(m0.XAxis(), m0.YAxis(), false, pBoneConstraintLookAt->m_lUpFlip != 0);
                    vcross(m0.ZAxis(), m0.XAxis(), m0.YAxis());
                    vnorm(m0.ZAxis());
                }
                else if (pBoneConstraintLookAt->m_lUpBoneAlignmentAxis == 0)
                {
                    projectUp(m0.ZAxis(), m0.YAxis(), true, pBoneConstraintLookAt->m_lUpFlip == 0);
                    vcross(m0.XAxis(), m0.YAxis(), m0.ZAxis());
                    vnorm(m0.XAxis());
                }
            }
            else if (pBoneConstraintLookAt->m_lLookAtAxis == 6)
            {
                m0.XAxis() = vLookAt;

                if (pBoneConstraintLookAt->m_lUpBoneAlignmentAxis == 3)
                {
                    projectUp(m0.YAxis(), m0.XAxis(), false, pBoneConstraintLookAt->m_lUpFlip != 0);
                    vcross(m0.ZAxis(), m0.XAxis(), m0.YAxis());
                    vnorm(m0.ZAxis());
                }
                else if (pBoneConstraintLookAt->m_lUpBoneAlignmentAxis == 0)
                {
                    projectUp(m0.ZAxis(), m0.XAxis(), true, pBoneConstraintLookAt->m_lUpFlip == 0);
                    vcross(m0.YAxis(), m0.ZAxis(), m0.XAxis());
                    vnorm(m0.YAxis());
                }
            }

            const ZVector3 vOldZ = m0.ZAxis();
            m0.ZAxis() = m0.YAxis();
            m0.YAxis() = vOldZ;
            vneg(m0.YAxis());
        }
    }

    void ZBoneModifyBase::LoadSave(ISerializerStream& stream, bool bSaving)
    {
        // TODO: Finish me
    }

    void ZBoneModifyBase::UpdateGlobalIK(ZBone* pBones, uint32_t lPrim, ZLNKOBJ* pLnkObj)
    {
        if (m_fGlobalScale != 1.0f && m_lNumActiveBones > 1)
        {
            for (uint32_t i = 1; i < m_lNumActiveBones; ++i)
            {
                vscalar(pBones[i]._Mat.XAxis(), m_fGlobalScale);
                vscalar(pBones[i]._Mat.YAxis(), m_fGlobalScale);
                vscalar(pBones[i]._Mat.ZAxis(), m_fGlobalScale);
            }
        }

        if (m_pDynamicsExt || (m_pRagdoll && m_pRagdoll->IsActive()) || m_bPassive)
        {
            if (m_pRagdoll && m_pRagdoll->IsActive())
            {
                m_pRagdoll->HandleCalcMatsMsg(pBones, false, pLnkObj->Model()->m_BoneCount);
                UpdateConstraintBones(pBones, lPrim, pLnkObj);
                return;
            }

            if (m_pDynamicsExt && m_pDynamicsExt->Awake(m_wBody))
            {
                pLnkObj->Model()->ResetBones();
                pLnkObj->Model()->ModelSpaceBones();

                m_pDynamicsExt->Update(m_wBody, pLnkObj);
            }
        }

        UpdateConstraintBones(pBones, lPrim, pLnkObj);
    }

    void ZBoneModifyBase::UpdateConstraintBones(ZBone* pBones, uint32_t lPrim, ZLNKOBJ* pLnkObj)
    {
        auto* pBoneConstraintsHeader = ZPrimControlBase::Instance()->GetBoneConstraints(lPrim);
        if (!pBoneConstraintsHeader)
            return;

        if (!pBoneConstraintsHeader->m_lNrConstraints)
            return;

        const auto lTotalConstraints = pBoneConstraintsHeader->m_lNrConstraints;
        auto* pConstraint = pBoneConstraintsHeader->Get();

        for (int lConstraintIndex = 0; lConstraintIndex < lTotalConstraints; ++lConstraintIndex)
        {
            if (pConstraint->m_lBoneIndex < m_lNumActiveBones)
            {
                if (pConstraint->m_lType != ZBoneConstraintLookAt::Type)
                {
                    ZASSERT(false);
                }
                else
                {
                    auto* pLookAt = pConstraint->As<ZBoneConstraintLookAt>();
                    CalculateLookAtMatrix(pBones[pConstraint->m_lBoneIndex]._Mat, pBones, pLookAt);
                }
            }

            pConstraint = pConstraint->Next();
        }
    }

    bool ZBoneModifyBase::DoAnimations() const
    {
        return !m_pDynamicsExt && (!m_pRagdoll || !m_pRagdoll->IsActive()) && !m_bPassive;
    }

    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(int32_t, lDecalLookup, 0x008EBE58, 0);
}
