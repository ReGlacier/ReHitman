#include <Glacier/Render/Prim/ZPrimControlBase.h>
#include <Glacier/Physics/CRagdoll2.h>
#include <Glacier/IK/ZLNKOBJ.h>


namespace Glacier
{
    void CRagdoll2::LoadSave(ISerializerStream& stream, bool bSaving)
    {
        // TODO: Finish me
    }

    void CRagdoll2::InitIndices()
    {
        m_nPelvisIx = GetBoneIndex(eBoneID::PELVIS);
        m_nSpineIx = GetBoneIndex(eBoneID::SPINE);
        m_nSpine1Ix = GetBoneIndex(eBoneID::SPINE_1);
        m_nNeckIx = GetBoneIndex(eBoneID::NECK);
        m_nHeadIx = GetBoneIndex(eBoneID::HEAD);
        m_nLClavicleIx = GetBoneIndex(eBoneID::LEFT_CLAVICLE);
        m_nLUpperArmIx = GetBoneIndex(eBoneID::LEFT_UPPER_ARM);
        m_nLLowerArmIx = GetBoneIndex(eBoneID::LEFT_FOREARM);
        m_nLHandIx = GetBoneIndex(eBoneID::LEFT_HAND);
        m_nRClavicleIx = GetBoneIndex(eBoneID::RIGHT_CLAVICLE);
        m_nRUpperArmIx = GetBoneIndex(eBoneID::RIGHT_UPPER_ARM);
        m_nRLowerArmIx = GetBoneIndex(eBoneID::RIGHT_FOREARM);
        m_nRHandIx = GetBoneIndex(eBoneID::RIGHT_HAND);
        m_nLThighIx = GetBoneIndex(eBoneID::LEFT_THIGH);
        m_nLCalfIx = GetBoneIndex(eBoneID::LEFT_CALF);
        m_nLFootIx = GetBoneIndex(eBoneID::LEFT_FOOT);
        m_nRThighIx = GetBoneIndex(eBoneID::RIGHT_THIGH);
        m_nRCalfIx = GetBoneIndex(eBoneID::RIGHT_CALF);
        m_nRFootIx = GetBoneIndex(eBoneID::RIGHT_FOOT);

        m_nLForeTwistIx = m_pLnkObj->GetBoneNrFromName("LEFT_FORETWIST");
        m_nRForeTwistIx = m_pLnkObj->GetBoneNrFromName("RIGHT_FORETWIST");
    }
    
    bool CRagdoll2::IsMoving() const
    {
        // TODO: Finish me
        return false;
    }

    void CRagdoll2::PrimChanged(uint32_t lPrim)
    {
        InitIndices();

        m_pJesusLocalBones = reinterpret_cast<const ZBone*>(ZPrimControlBase::Instance()->GetLocalPrimBones(lPrim));
    }

    bool CRagdoll2::IsRagdoll() const
    {
        return m_bRagdoll;
    }

    bool CRagdoll2::IsActive() const
    {
        return m_bActive;
    }

    uint16_t CRagdoll2::GetBoneIndex(eBoneID eBone)
    {
        return m_pLnkObj->GetBoneNrFromId(eBone);
    }
}