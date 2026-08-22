#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/ZRenderBaseDll.h>
#include <Glacier/Render/Draw/IDraw.h>
#include <Glacier/Render/Globals.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/IK/ZBoneModifyBase.h>
#include <Glacier/Physics/ZRagdollContainer.h>
#include <Glacier/Physics/ZDynamicsExtend.h>
#include <Glacier/Physics/CRagdoll2.h>
#include <Glacier/Animation/Model.h>
#include <Glacier/Animation/ZBone.h>
#include <Glacier/IK/ZLNKOBJ.h>



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

    ZBoneModifyBase::ZBoneModifyBase(uint16_t lNrBones)
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

    void ZBoneModifyBase::GetBoneMatPos(ZMat3x3& mMat, ZVector3& vPos, uint32_t lBoneIdx, const ZLNKOBJ* pLnkObj) const
    {
        ZBone bone {};

        const auto* pLUT = ZPrimControlBase::Instance()->GetBoneIdToIndexLookup(pLnkObj->Prim());
        if (!GetIKBoneMatPos(mMat, vPos, pLUT[lBoneIdx], pLnkObj, &bone))
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
            ZERROR("ZBoneModifyBase::Update twice in same frame");
            return false;
        }

        const float fDt = g_pSysInterface->FrameTime - m_fLastUpdateTime;
        m_lLastUpdateFrameCount = g_pSysInterface->m_lFrameCount;
        m_fLastUpdateTime = g_pSysInterface->FrameTime;

        if (m_pRagdoll && m_pRagdoll->IsActive())
        {
            m_bPassive = false;
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
                    return true;
                }

                if (!m_pRagdoll || !m_pRagdoll->IsActive() || m_pRagdoll->Move(mMat, vPos, fDt))
                {
                    return true;
                }

                ForceRagdollDeactivation(pLnkObj);
            }
        }

        return false;
    }

    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(int32_t, lDecalLookup, 0x008EBE58, 0);
}
