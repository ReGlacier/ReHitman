#include <Glacier/EventBase/ZPhysicsLinkage.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/IK/ZBoneModifyBase.h>
#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/Physics/ZBoneExtend.h>
#include <Glacier/Physics/ZDynamicsExtend.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZMessageResolver.h>

namespace Glacier
{
    namespace
    {
        ZMessageResolver g_msgProjectileHit("ProjectileHit");
        ZMessageResolver g_msgFracture("Fracture");
        ZMessageResolver g_msgReleaseLinkage("ReleaseLinkage");
    }

    ZPhysicsLinkage::ZPhysicsLinkage()
        : CBaseEvent<ZGEOM>()
        , m_nBoneLink(0)
        , m_Rot()
        , m_Pos()
        , m_pRB(nullptr)
        , m_Status(0)
        , m_verify(0)
        , m_sName(NAME)
    {
    }

    ZPhysicsLinkage::~ZPhysicsLinkage()
    {
        ZUniMemory::Delete(m_pRB);
    }

    const RTP::ZPropertyInfo& ZPhysicsLinkage::GetProperties() const
    {
        return ZPhysicsLinkage::Info;
    }

    bool ZPhysicsLinkage::PostLoad(ISerializerStream& stream)
    {
        ZEventBase::PostLoad(stream);
        if (geom_cast<ZLNKOBJ>(ZGEOM::RefToPtr(m_verify)))
            m_Status |= eVALID;
        else
            m_Status &= static_cast<uint8_t>(~eVALID);
        return true;
    }

    void ZPhysicsLinkage::Update(const ZBone* pBone)
    {
        if (!(m_Status & eVALID) || !pBone || !m_pBaseGeom)
            return;

        ZMat3x3 mat;
        ZVector3 pos;
        mmmul(mat, m_Rot, pBone->_Mat);
        vmmul(pos, m_Pos, pBone->_Mat);
        pos += pBone->_Pos;

        auto* pLinkObject = geom_cast<ZLNKOBJ>(ZGEOM::RefToPtr(m_verify));
        if (pLinkObject)
            pLinkObject->GetRootMatPos(mat, pos);

        if (auto* pParent = m_pBaseGeom->BaseGeom()->ParentGroup())
            pParent->GetLocalMatPos(mat, pos);

        if (m_Status & ePARTICLELINK)
            m_pBaseGeom->SetRootTM(mat, pos);
        else
            m_pBaseGeom->SetMatPos(mat, pos);

        if (!(m_Status & eRIGIDBODYLINK) || !m_pRB)
            return;

        if (vlen2(m_pRB->m_LastPos[1] - pBone->_Pos) >= 0.01f
            || vlen2(m_pRB->m_LastPos[0] - pBone->_Pos) >= 0.01f
            || vlen2(m_pRB->m_LastPos[0] - pos) >= 0.01f)
        {
            m_pRB->m_LastMat[0] = m_pRB->m_LastMat[1];
            m_pRB->m_LastMat[1] = mat;
        }

        if (vlen2(m_pRB->m_LastPos[1] - pos) >= 1.0f)
        {
            m_pRB->m_LastPos[0] = m_pRB->m_LastPos[1];
            m_pRB->m_LastPos[1] = pos;
        }
    }

    void ZPhysicsLinkage::FrameUpdate()
    {
        const auto* pLinkObject = geom_cast<ZLNKOBJ>(ZGEOM::RefToPtr(m_verify));
        if (!pLinkObject)
            return;

        const auto* pModifier = pLinkObject->GetBoneModifier();
        if (pModifier)
            Update(pModifier->GetBones(pLinkObject));
    }

    void ZPhysicsLinkage::ReleaseLinkage()
    {
        m_Status &= static_cast<uint8_t>(~(ePARTICLELINK | eRIGIDBODYLINK | eCLOTHLINK | eCONNECTED));
        ZUniMemory::Delete(m_pRB);
        m_pRB = nullptr;
        DeactivateFrameUpdate();
    }

    int ZPhysicsLinkage::Command(ZMSGID command, ZDATA data)
    {
        if (!(m_Status & eVALID))
            return 0;

        if (command == static_cast<ZMSGID>(g_msgProjectileHit) && data && (m_Status & ePBALINKOBJ))
        {
            if (auto* pLinkObject = geom_cast<ZLNKOBJ>(ZGEOM::RefToPtr(m_verify)))
                pLinkObject->SendCommand(command, data, nullptr);
        }
        else if (command == static_cast<ZMSGID>(ZDynamicsExtend::m_msgFracture) && data)
        {
            const auto* fracture = static_cast<const SBodyFracture*>(data);
            if (fracture->frac_obj == m_verify)
            {
                if (fracture->frac_bone == m_nBoneLink)
                    ReleaseLinkage();
                else
                    m_Status &= static_cast<uint8_t>(~eRIGIDBODYLINK);
            }
        }
        else if (command == static_cast<ZMSGID>(g_msgReleaseLinkage))
        {
            ReleaseLinkage();
        }
        return 0;
    }

#   pragma region " --- RTTI --- "
    DEFINE_ROUT_CLASS(ZPhysicsLinkage, ZGEOM, PhysicsLinkage, 0, 0, 0, nullptr, ZGEOM);
#   pragma endregion
}
