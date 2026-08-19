#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Render/Draw/IDraw.h>
#include <Glacier/IK/ZBoneModifyBase.h>
#include <Glacier/Physics/CRagdoll2.h>
#include <Glacier/Animation/ZBone.h>
#include <Glacier/IK/ZLNKOBJ.h>



namespace Glacier
{
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

    bool ZBoneModifyBase::HideBone(ZBaseGeom* pBaseGeom, bool, bool)
    {
        // TODO: Finish me
        return false;
    }

    void ZBoneModifyBase::GetIKBone(const ZBone* pBones, const float* pConvBones, uint32_t lBoneIndex, Matrix3x3& mMat, Vector3& vPos)
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

        // TODO: Finish missing assert
        // Smth about Animation::Model entry
        return nullptr;
    }

    const SRagdollCollisionInfo* ZBoneModifyBase::GetCollisionInfo() const
    {
        return m_pRagdoll ? &m_pRagdoll->m_ColiInfo : nullptr;
    }
    
    void ZBoneModifyBase::GetBoneMatPos(ZMat3x3& mMat, ZVector3& vPos, uint32_t lBoneIdx, const ZLNKOBJ* pLnkObj)
    {
        ZBone bone {};

        const auto* pLUT = ZPrimControlBase::Instance()->GetBoneIdToIndexLookup(pLnkObj->Prim());
        if (!GetIKBoneMatPos(mMat, vPos, pLUT[lBoneIdx], pLnkObj, &bone))
        {
            mMat.Reset();
            vPos.Reset();
        }
    }

    bool ZBoneModifyBase::GetIKBoneMatPos(ZMat3x3& mMat, ZVector3& vPos, uint8_t lIndex, const ZLNKOBJ* pLnkObj, ZBone* pBone)
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
        ZBone aBones[256];
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
}
