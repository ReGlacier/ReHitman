#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/Draw/IDraw.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Render/Prim/ZBoneConstraintsHeader.h>
#include <Glacier/Render/Prim/ZBoneConstraintLookAt.h>
#include <Glacier/Render/Prim/SBoneDefinition.h>
#include <Glacier/IK/ZBoneModifyBase.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/IK/ZLNKOBJ.h>
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

    bool ZBoneModifyBase::CalcShadowProjectPlane(const ZLNKOBJ* pLnkObj, float* vTans, const float* mObjectToLight, const float* pObjectToLight) const
    {
        const uint32_t prim = pLnkObj->Prim();
        if (!ZPrimControlBase::Instance()->GetConvBones(prim))
            return false;

        const auto* lookup = ZPrimControlBase::Instance()->GetBoneIdToIndexLookup(prim);
        const auto* definitions = ZPrimControlBase::Instance()->GetBoneDefinitions(prim);
        const ZBone* bones = GetBones(pLnkObj);
        const auto& lightMat = *reinterpret_cast<const ZMat3x3*>(mObjectToLight);
        const auto& lightPos = *reinterpret_cast<const ZVector3*>(pObjectToLight);

        float minX = 9.9999997e37f;
        float maxX = -9.9999997e37f;
        float minY = 9.9999997e37f;
        float maxY = -9.9999997e37f;

        const auto addBox = [&](const ZMat3x3& mat, const ZVector3& pos, const ZVector3& center, const ZVector3& size)
        {
            ZVector3 localCenter;
            vmmul(localCenter, center, mat);
            localCenter += pos;

            ZMat3x3 projectedMat;
            mmmul(projectedMat, mat, lightMat);
            ZVector3 projectedCenter;
            vmmul(projectedCenter, localCenter, lightMat);
            projectedCenter += lightPos;

            for (uint32_t corner = 0; corner < 8; ++corner)
            {
                ZVector3 point(
                    (corner & 1) ? size.x : -size.x,
                    (corner & 2) ? size.y : -size.y,
                    (corner & 4) ? size.z : -size.z);
                TransformRootVector(point, projectedMat);
                point += projectedCenter;
                if (point.z > 0.0f)
                {
                    const float inverseZ = 1.0f / point.z;
                    minX = std::min(minX, point.x * inverseZ);
                    maxX = std::max(maxX, point.x * inverseZ);
                    minY = std::min(minY, point.y * inverseZ);
                    maxY = std::max(maxY, point.y * inverseZ);
                }
            }
        };

        constexpr uint8_t shadowBoneIds[10] = { 23, 24, 21, 22, 18, 17, 14, 13, 2, 1 };
        for (uint8_t boneId : shadowBoneIds)
        {
            const uint8_t boneIndex = lookup[boneId];
            ZASSERT(boneIndex != 0xFF);
            addBox(bones[boneIndex]._Mat, bones[boneIndex]._Pos, definitions[boneIndex].Center, definitions[boneIndex].Size);
        }

        for (uint32_t i = 0; i < m_AttachedGeoms.Count(); ++i)
        {
            const ZAttachGeom* attached = m_AttachedGeoms.Get(i);
            ZGEOM* geom = ZGEOM::RefToPtr(attached->m_rBaseGeom);
            if (!geom)
                continue;

            ZMat3x3 mat;
            ZVector3 pos;
            if (!pLnkObj->GetAttachedGeomMatPos(geom->BaseGeom(), mat, pos))
                continue;

            ZVector3 center;
            ZVector3 size;
            geom->GetCen(center);
            geom->GetSize(size);
            addBox(mat, pos, center, size);
        }

        vTans[0] = std::min(minX, 0.0f);
        vTans[1] = std::max(maxX, 0.0f);
        vTans[2] = std::min(minY, 0.0f);
        vTans[3] = std::max(maxY, 0.0f);
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
        stream.Exchange("m_lNumActiveBones", m_lNumActiveBones);

        uint32_t attachedGeomCount = m_AttachedGeoms.Count();
        stream.Exchange("lNrAttachedGeoms", attachedGeomCount);
        stream.Exchange("m_bPassive", m_bPassive);
        stream.Exchange("m_bIsPlayer", m_bIsPlayer);
        stream.Exchange("m_lHiddenBoneIds", m_lHiddenBoneIds);
        stream.Exchange("m_fGlobalScale", m_fGlobalScale);

        if (bSaving)
        {
            for (uint32_t i = 0; i < m_AttachedGeoms.Count(); ++i)
            {
                ZAttachGeom& attachGeom = *m_AttachedGeoms.Get(i);
                stream.Exchange("m_rBaseGeom", attachGeom.m_rBaseGeom);
                stream.Exchange("m_lBoneId", attachGeom.m_lBoneId);
                stream.ExchangeArray("m_vOffset", attachGeom.m_vOffset, 3);
                stream.ExchangeArray("m_mOffset", attachGeom.m_mOffset, 9);
            }
        }
        else
        {
            m_AttachedGeoms.Clear();
            for (uint32_t i = 0; i < attachedGeomCount; ++i)
            {
                ZAttachGeom attachGeom{};
                stream.Exchange("m_rBaseGeom", attachGeom.m_rBaseGeom);
                stream.Exchange("m_lBoneId", attachGeom.m_lBoneId);
                stream.ExchangeArray("m_vOffset", attachGeom.m_vOffset, 3);
                stream.ExchangeArray("m_mOffset", attachGeom.m_mOffset, 9);
                m_AttachedGeoms.Add(&attachGeom);
            }
        }

        bool useRagdoll = m_pRagdoll && m_pRagdoll->m_bActive;
        stream.Exchange("bUseRagdoll", useRagdoll);
        if (useRagdoll)
        {
            bool isDragdoll = m_pRagdoll && m_pRagdoll->m_bRagdoll;
            stream.Exchange("bIsDragdoll", isDragdoll);
            if (!bSaving)
                m_pRagdoll = g_pRenderDll->GetRagdollContainer()->GetRagdoll(isDragdoll);
            m_pRagdoll->LoadSave(stream, bSaving);
        }

        stream.Exchange("m_fAimBlendSpeed", m_fAimBlendSpeed);
    }

    bool ZBoneModifyBase::ActivateRagdoll(ZLNKOBJ* pLnkObj, bool bActive, bool bEnableTimeout, bool bUseDamping)
    {
        if (m_lNumActiveBones > 0x100u)
            return false;

        if (!bActive)
        {
            if ((m_pRagdoll && m_pRagdoll->IsActive()) || m_bPassive)
            {
                pLnkObj->ResetInactiveBones();
                if (pLnkObj->BaseGeom()->m_lDrawId)
                {
                    auto* pBones = const_cast<ZBone*>(reinterpret_cast<const ZBone*>(pLnkObj->GetBones()));
                    IDraw::Instance()->CreateDefaultBones(pBones, pLnkObj);
                }
            }

            if (m_pRagdoll)
            {
                g_pRenderDll->GetRagdollContainer()->DeactivateRagdoll(m_pRagdoll);
                m_pRagdoll = nullptr;
            }

            m_bPassive = true;
            return true;
        }

        if (m_pRagdoll)
        {
            m_pRagdoll->EnableTimeOut(bEnableTimeout);

            const float fDamp = bUseDamping ? 0.15f : 0.009f;
            m_pRagdoll->SetDamping(fDamp);
            if (!m_pRagdoll->IsActive())
            {
                m_pRagdoll->Activate(GetBones(pLnkObj), true);
            }
            return true;
        }
        else
        {
            auto* pRagdollContainer = g_pRenderDll->GetRagdollContainer();
            m_pRagdoll = pRagdollContainer->GetRagdoll(bUseDamping);

            if (!m_pRagdoll)
            {
                return false;
            }

            if (GetBones(pLnkObj) && (pLnkObj->Model() && pLnkObj->Model()->m_Valid))
            {
                auto lBoneCount = pLnkObj->Model()->m_BoneCount;
                const ZBone* pBones = GetBones(pLnkObj);

                m_pRagdoll->Init(pLnkObj, pBones, lBoneCount);
            }
            else
            {
                const auto* pBones = reinterpret_cast<const ZBone*>(ZPrimControlBase::Instance()->GetGlobalPrimBones(pLnkObj->Prim()));
                auto lNrBones = ZPrimControlBase::Instance()->GetNrBones(pLnkObj->Prim());
                m_pRagdoll->Init(pLnkObj, pBones, lNrBones);
            }

            auto* pBones = GetBones(pLnkObj);
            m_pRagdoll->Activate(pBones, false);
            pLnkObj->OnMoving();
            m_pRagdoll->EnableTimeOut(bEnableTimeout);
            m_bPassive = false;

            const float fDamp = bUseDamping ? 0.15f : 0.009f;
            m_pRagdoll->SetDamping(fDamp);
            return true;
        }

        return false;
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
