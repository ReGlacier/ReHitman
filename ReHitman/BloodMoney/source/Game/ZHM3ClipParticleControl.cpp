#include <BloodMoney/Game/ZHM3ClipParticleControl.h>

#include <BloodMoney/Game/Items/ZHM3ItemWeapon.h>
#include <BloodMoney/Game/Physics/CRigidBody.h>
#include <BloodMoney/Game/ZHitman3.h>
#include <BloodMoney/Game/ZHM3GameData.h>

#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Data/ZGameData.h>
#include <Glacier/GameBase/ZActor.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/Physics/SRigidBodyVelocity.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ZUniAssert.h>

namespace Hitman
{
    // PC 0x64CED0. The weapon must be a ZHM3ItemWeapon; when its owner is an actor the casing is
    // only spawned if the weapon is in hand and the actor is within 2000 units (squared 4000000)
    // of the player, otherwise the owner is non-actor geometry and the casing always spawns.
    void ZHM3ClipParticleControl::SpawnClipAtMatPosSpeed(
        const Glacier::ZMat3x3& mat,
        const Glacier::ZVector3& pos,
        const Glacier::ZVector3& speed,
        ZHM3ItemWeapon* pWeapon,
        bool bSixShooter)
    {
        ZASSERT(pWeapon != nullptr);
        ZASSERT((ZHM3ItemWeapon::m_Mask & pWeapon->GetObjectId()) == ZHM3ItemWeapon::m_Id);

        Glacier::ZGEOM* pOwner = pWeapon->GetItemOwner();
        if (pOwner == nullptr)
            return;

        if ((Glacier::ZActor::m_Mask & pOwner->GetObjectId()) == Glacier::ZActor::m_Id)
        {
            if ((pOwner->BaseGeom()->m_lControl & 0x1000) == 0)
                return;

            auto* pGameData = static_cast<ZHM3GameData*>(Glacier::g_pGameData);
            ZASSERT(pGameData != nullptr);

            Glacier::ZVector3 vPlayerPos;
            Glacier::ZVector3 vOwnerPos;
            pGameData->m_Hitman3->GetRootPoint(vPlayerPos);
            pOwner->GetRootPoint(vOwnerPos);
            if (Glacier::vdist2(vPlayerPos.Get(), vOwnerPos.Get()) > 4000000.0f)
                return;
        }

        auto* pShell = static_cast<Glacier::ZGEOM*>(AllocOne(Glacier::g_pEngineData->m_pRoot, true));
        if (pShell == nullptr)
            return;

        // Reset every sub-geom of the freshly popped casing back to the origin.
        for (Glacier::ZBaseGeom* pBase = pShell->BaseGeom(); pBase != nullptr; pBase = pBase->Next())
        {
            Glacier::ZMat3x3 matReset;
            Glacier::ZVector3 posReset;
            Glacier::mreset(matReset.Get());
            Glacier::vreset(posReset.Get());
            pBase->SetMatPos(matReset.Get(), posReset.Get());
        }

        pShell->SetRootTM(mat, pos);
        pShell->MakeActive();
        pShell->MakeDynamic(true);
        pShell->SetAutoRoomAssign(true);

        auto* pRigidBody = static_cast<CRigidBody*>(pShell->FindEvent(CRigidBody::Name));
        if (pRigidBody == nullptr)
            pRigidBody = static_cast<CRigidBody*>(pShell->AddEvent(CRigidBody::ClassName));

        const Glacier::ZMSGID lSetVelocity = Glacier::g_pEngineData->RegisterZMsg("SetVelocity", 0, __FILE__, __LINE__);

        // The casing is launched by reporting an old position ahead of the current one: SetVelocity
        // derives its impulse from (m_vOldPos - m_vPos).
        Glacier::SRigidBodyVelocity velocity{};
        velocity.m_mOldMat = mat;
        velocity.m_mMat = mat;
        velocity.m_vOldPos = pos;
        Glacier::vaddscalar(velocity.m_vOldPos.Get(), velocity.m_vOldPos.Get(), speed.Get(), bSixShooter ? 1.3f : 5.0f);
        velocity.m_vPos = pos;
        velocity.m_fTime = 0.1f;

        pShell->SendCommandRecursive(m_msgActivate, nullptr, nullptr);
        pShell->SendCommandRecursive(lSetVelocity, &velocity, nullptr);

        pRigidBody->m_bDoNotAddSoundEvent = true;
    }
}
