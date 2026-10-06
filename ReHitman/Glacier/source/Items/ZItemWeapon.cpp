#include <Glacier/Items/ZItemWeapon.h>

#include <BloodMoney/Engine/ZHM3Camera.h>
#include <BloodMoney/Game/ZHitman3.h>

#include <Glacier/Com/CCom.h>
#include <Glacier/CProjectileActivate.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/EventBase/ZEventBase.h>
#include <Glacier/EventBase/ZEventBuffer.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZAllocMany.h>
#include <Glacier/Geom/ZParticleController.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/IK/ZCTRLIKLNKOBJ.h>
#include <Glacier/Items/ZItem.h>
#include <Glacier/Items/ZItemTemplate.h>
#include <Glacier/Items/ZItemTemplateAmmo.h>
#include <Glacier/Items/ZItemTemplateWeapon.h>
#include <Glacier/RTP/Base.h>
#include <Glacier/RTP/PropertyTypes.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/Runtime/ZGEOMCLASSINFO.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZSTL/ZMath.h>
#include <cstdint>


namespace Glacier
{
    // The PC ctor (0x50F6C0) registers the "Activate" / "WeaponFired" engine messages into globals
    // (0x99BF20 / 0x99BF24); the class has no message members, so mirror them as TU statics.
    static ZMSGID g_MSG_Activate = 0;
    static ZMSGID g_MSG_WeaponFired = 0;

    // Engine globals defined in this TU (declared in ZItemWeapon.h):
    //   g_bIsInfClip  PC 0x99BF28 -- "InfClip" cheat flag (ZCheatMenu MENU_TOGGLE_BOOL) that makes
    //                                GetProjectilesInMagazine report 999.
    //   g_lBlockFire  PC 0x99BF2C -- debug gate that blocks FireRound entirely.
    ZDebugInt g_bIsInfClip { "InfClip", "Gives Hero infinite ammo and doesn't make him change the clip", 0, 0, 1, 1, "Hitman/" };
    ZDebugInt g_lBlockFire { "BlockFire", "Blocks fire from all weapons", 0, 0, 1, 1, "Weapons/" };

    // Animation commands sent to m_rSlide / m_rClip, e.g. SendCommand(2058, &value, 0).
    static constexpr ZMSGID ZMSG_ANIM_2058 = 2058;
    static constexpr ZMSGID ZMSG_ANIM_2059 = 2059;

    // vtbl slot 0 (constructor)  PC 0x50F6C0
    ZItemWeapon::ZItemWeapon(const char* psName, ZBaseGeom* pBaseGeom)
        : ZItem(psName, pBaseGeom)
    {
        m_bWantSoundEvent = true;                          // [ +0xD9 ] = 1
        m_lProjectilesInMagazine = 0;                      // [ +0x94 ] = 0
        m_bTriggerHeld = false;                            // [ +0xA3 ] = 0
        m_bReloading = false;                              // [ +0xA1 ] = 0
        m_bChambering = false;                             // [ +0xA2 ] = 0
        m_bRequestFireRelease = false;                     // [ +0xA0 ] = 0
        m_lBurstCount = 0;                                 // dword [ +0x9C ] = 0
        m_vTarget = ZVector3(0.0f, 0.0f, 0.0f);            // [ +0x84 ] = 0
        m_fTimeLastShot = 0.0f;                            // [ +0xA4 ] = 0
        m_vMuzzleLightAlign = ZVector3(0.0f, 0.0f, 0.0f);  // [ +0xC4 ] = 0

        g_MSG_Activate = g_pEngineData->RegisterZMsg("Activate", 0, __FILE__, __LINE__);
        g_MSG_WeaponFired = g_pEngineData->RegisterZMsg("WeaponFired", 0, __FILE__, __LINE__);

        m_prtWeaponParts = REFTAB::MakeReftab(8, 0);       // [ +0xAC ]
        m_useBulletsFromMagazine = true;                   // [ +0xD8 ] = 1

        // NOTE: like the PC ctor, m_rAmmoTemplate, m_bBulletInChamber, m_eWeaponOperation,
        //       m_rParticleController, m_lMuzzleFireIndex/Smoke/CartridgeIndex, m_rMuzzleLight,
        //       m_rSlide and m_rClip are intentionally left untouched (they are filled by RTP load).
    }

    // vtbl slot 0 (deleting dtor 0x512070 -> body 0x50F870)
    ZItemWeapon::~ZItemWeapon()
    {
        // Delete the referenced muzzle-light geometry.
        if (ZGEOM* pMuzzleLight = ZGEOM::RefToPtr(m_rMuzzleLight))
            pMuzzleLight->Delete();

        // Delete every geometry referenced by the weapon-parts table, then the table itself.
        if (m_prtWeaponParts != nullptr)
        {
            RefRun Run;
            m_prtWeaponParts->RunInitNxtRef(&Run);
            for (uint32_t* pRef = m_prtWeaponParts->RunNxtRefPtr(&Run);
                 pRef != nullptr;
                 pRef = m_prtWeaponParts->RunNxtRefPtr(&Run))
            {
                ZGEOM* pPart = ZGEOM::RefToPtr(*pRef);
                ZASSERT(pPart != nullptr);
                if (pPart != nullptr)
                    pPart->Delete();
            }

            REFTAB::DeleteReftab(m_prtWeaponParts);
            m_prtWeaponParts = nullptr;
        }
    }

    // vtbl slot 12  PC 0x50F810
    const RTP::ZPropertyInfo& ZItemWeapon::GetProperties() const
    {
        return ZItemWeapon::Info;
    }

    // vtbl slot 13  PC 0x510AC0
    uint32_t ZItemWeapon::GetObjectId() const
    {
        return ZItemWeapon::m_Id;
    }

    // vtbl slot 14  PC 0x510AD0
    void ZItemWeapon::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZItemWeapon::m_Id;
        mask = ZItemWeapon::m_Mask;
    }

    // vtbl slot 15  PC 0x50F820
    ZGEOMCLASSINFO* ZItemWeapon::GetOldClassInfo() const
    {
        return ZItemWeapon::m_OldClassInfo;
    }

    // vtbl slot 104  PC 0x512090 (ZItem::CopyData is inlined by the compiler on PC)
    void ZItemWeapon::CopyData(const ZGEOM* Source)
    {
        ZItem::CopyData(Source);

        if ((Source->GetObjectId() & ZItemWeapon::m_Mask) == ZItemWeapon::m_Id)
        {
            m_rAmmoTemplate = static_cast<const ZItemWeapon*>(Source)->m_rAmmoTemplate;
        }
    }

    // vtbl slot 173  PC 0x50F830
    void ZItemWeapon::DestroyItem()
    {
        GetWeaponTemplate()->DestroyItem(this);
    }

    // vtbl slot 174  PC 0x512570
    void ZItemWeapon::SetAmmoTemplate(ZItemTemplateAmmo* pAmmoTemplate)
    {
        if (pAmmoTemplate != nullptr &&
            (pAmmoTemplate->GetObjectId() & ZItemTemplateAmmo::m_Mask) != ZItemTemplateAmmo::m_Id)
        {
            ZASSERT(false);
        }

        m_rAmmoTemplate = pAmmoTemplate != nullptr ? pAmmoTemplate->GetRef() : 0;
    }

    // vtbl slot 175  PC 0x510050
    ZItemTemplateAmmo* ZItemWeapon::GetAmmoTemplate()
    {
        return static_cast<ZItemTemplateAmmo*>(ZGEOM::RefToPtr(m_rAmmoTemplate));
    }

    // vtbl slot 176  PC 0x50FB10
    void ZItemWeapon::SetTarget(const Glacier::ZVector3* target)
    {
        m_vTarget = *target;
    }

    // vtbl slot 177  PC 0x50FDA0
    bool ZItemWeapon::GetTarget(Glacier::ZVector3* targetPos)
    {
        *targetPos = m_vTarget;
        return m_vTarget.x != 0.0f || m_vTarget.y != 0.0f || m_vTarget.z != 0.0f;
    }

    // vtbl slot 178  PC 0x64A2C0 (thunk)
    void* ZItemWeapon::Target()
    {
        return &m_vTarget;
    }

    // vtbl slot 179  PC 0x5BBB60
    WEAPONOPERATION ZItemWeapon::GetWeaponOperation()
    {
        return m_eWeaponOperation;
    }

    // vtbl slot 180  PC 0x50FB40
    void ZItemWeapon::SetWeaponOperation(WEAPONOPERATION weaponOperation)
    {
        m_eWeaponOperation = weaponOperation;
    }

    // vtbl slot 181  PC 0x50FB50
    void ZItemWeapon::SelectNextWeaponOperation()
    {
        m_eWeaponOperation = GetWeaponTemplate()->SelectNextWeaponOperation(m_eWeaponOperation);
    }

    // vtbl slot 182  PC 0x50F350
    void ZItemWeapon::GetFirePosition(ZMat3x3* mat, ZVector3* pos)
    {
        GetMuzzleExitPos()->GetMatPos(*mat, *pos);
    }

    // vtbl slot 183  PC 0x50F360
    ZGEOM* ZItemWeapon::GetMuzzleExitPos()
    {
        return GetWeaponTemplate()->m_pProjectileAlign;
    }

    // vtbl slot 184  PC 0x50F370
    void ZItemWeapon::GetRootMuzzleExitPos(const Glacier::ZVector3* result)
    {
        ZVector3* pPosition = const_cast<ZVector3*>(result);

        GetWeaponTemplate()->m_pProjectileAlign->GetPos(*pPosition);

        ZMat3x3 mRoot;
        ZVector3 vRoot;
        GetItemRootTM(reinterpret_cast<float*>(&mRoot), reinterpret_cast<float*>(&vRoot));

        TransformRootVector(*pPosition, mRoot);
        vadd(reinterpret_cast<float*>(pPosition), reinterpret_cast<const float*>(&vRoot));
    }

    // vtbl slot 185  PC 0x511EE0
    ZItemTemplateWeapon* ZItemWeapon::GetWeaponTemplate()
    {
        ZItemTemplate* pTemplate = GetItemTemplate();
        ZASSERT(pTemplate != nullptr);
        ZASSERT(pTemplate->IsDerivedFrom<ZItemTemplateWeapon>());
        return static_cast<ZItemTemplateWeapon*>(pTemplate);
    }

    // vtbl slot 186  PC 0x50F850
    bool ZItemWeapon::GetBulletInChamber()
    {
        return m_bBulletInChamber;
    }

    // vtbl slot 187  PC 0x50FAE0
    void ZItemWeapon::SetBulletInChamber(bool value)
    {
        m_bBulletInChamber = value;
    }

    // vtbl slot 188  PC 0x50FA00
    int ZItemWeapon::GetProjectilesInMagazine()
    {
        return g_bIsInfClip ? 999 : m_lProjectilesInMagazine;
    }

    // vtbl slot 189  PC 0x6D1350
    void ZItemWeapon::SetProjectilesInMagazine(int value)
    {
        m_lProjectilesInMagazine = value;
    }

    // vtbl slot 190  PC 0x50FA20
    int ZItemWeapon::GetProjectilesPerMagazine()
    {
        if (m_rItemTemplate != 0)
        {
            const int lDefault = GetWeaponTemplate()->GetDefaultProjectilesPerMagazine();
            if (lDefault != 0)
                return lDefault;

            if (GetAmmoTemplate() != nullptr)
                return GetAmmoTemplate()->GetDefaultProjectilesPerMagazine();
        }

        return -1;
    }

    // vtbl slot 191  PC 0x50FA70
    bool ZItemWeapon::WeaponReady()
    {
        if (!GetWeaponTemplate()->CanFireProjectiles())
            return true;

        if (!m_bBulletInChamber)
            return false;

        // PC: trunc((GetTimeBetweenShots() + m_fTimeLastShot) * 1024) <= g_pSysInterface->FrameTime
        const float fNextShot = GetWeaponTemplate()->GetTimeBetweenShots() + m_fTimeLastShot;
        const int lNextShot = static_cast<int>(fNextShot * 1024.0f);
        return lNextShot <= static_cast<int>(g_pSysInterface->FrameTime);
    }

    // vtbl slot 192  PC 0x50F860
    void ZItemWeapon::SetUseBulletsFromMagazine(bool value)
    {
        m_useBulletsFromMagazine = value;
    }

    // vtbl slot 193  PC 0x6EFF20
    int ZItemWeapon::GetNumOfBulletsPerReloadCycle()
    {
        return 1;
    }

    // vtbl slot 194  PC 0x514150
    void ZItemWeapon::ReloadStart()
    {
        SetState(IS_EXTRA1, nullptr);
        ReloadStartActivateAnimation();
    }

    // vtbl slot 195  PC 0x50FB80
    void ZItemWeapon::ReloadEnd()
    {
        SetBulletInChamber(true);

        const int lFrame = static_cast<int>(g_pSysInterface->FrameTime) + 102;
        const int lScaled = static_cast<int>(GetWeaponTemplate()->GetTimeBetweenShots() * 1024.0f);
        m_fTimeLastShot = static_cast<float>(lFrame - lScaled) * (1.0f / 1024.0f);
    }

    // vtbl slot 196  PC 0x514170
    void ZItemWeapon::ReloadStartActivateAnimation()
    {
        if (ZGEOM* pSlide = ZGEOM::RefToPtr(m_rSlide))
        {
            int lValue = 4;
            pSlide->SendCommand(ZMSG_ANIM_2058, &lValue, nullptr);
            lValue = -1;
            pSlide->SendCommand(ZMSG_ANIM_2059, &lValue, nullptr);
            pSlide->SendCommand(g_MSG_Activate, this, nullptr);
        }

        ZGEOM* pOwner = GetItemOwner();

        // The PC only falls back to animating m_rClip when the owner is either not a
        // ZCTRLIKLNKOBJ at all, or a human-controlled one. For computer-controlled owners
        // the actor animation drives the clip, so it is skipped here.
        const bool bAnimateClip =
            pOwner == nullptr ||
            (pOwner->GetObjectId() & ZCTRLIKLNKOBJ::m_Mask) != ZCTRLIKLNKOBJ::m_Id ||
            static_cast<ZCTRLIKLNKOBJ*>(pOwner)->GetController() == ZCTRLIKLNKOBJ::CONTROLLER_HUMAN;

        if (bAnimateClip)
        {
            if (ZGEOM* pClip = ZGEOM::RefToPtr(m_rClip))
            {
                if ((pClip->m_baseGeom->m_lControl & 0x400u) != 0)
                    pClip->MakeActive();

                int lValue = 4;
                pClip->SendCommand(ZMSG_ANIM_2058, &lValue, nullptr);
                lValue = -1;
                pClip->SendCommand(ZMSG_ANIM_2059, &lValue, nullptr);
                pClip->SendCommand(g_MSG_Activate, this, nullptr);
            }
        }
    }

    // vtbl slot 197  PC 0x5142A0
    void ZItemWeapon::ChamberStart()
    {
        if (GetOldClassInfo() != nullptr)
            SetState(IS_EXTRA2, nullptr);
    }

    // vtbl slot 198  PC 0x5142C0
    void ZItemWeapon::ChamberEnd()
    {
        if (GetOldClassInfo() != nullptr)
        {
            SetState(eIS_NORMAL, nullptr);
            m_bBulletInChamber = true;
        }
    }

    // vtbl slot 199  PC 0x5125B0
    void ZItemWeapon::FireStart()
    {
        m_bRequestFireRelease = false;

        if (GetAmmoTemplate() == nullptr)
            return;

        switch (m_eWeaponOperation)
        {
        case EWeaponOperation::WO_MANUAL:
        case EWeaponOperation::WO_SEMIAUTO:
            m_lBurstCount = 1;
            break;
        case EWeaponOperation::WO_FULLAUTO:
            m_lBurstCount = GetProjectilesInMagazine();
            break;
        case EWeaponOperation::WO_FULLAUTO3:
            m_lBurstCount = 3;
            break;
        default:
            break;
        }

        const int lAmmo = GetProjectilesInMagazine();
        if (m_lBurstCount > lAmmo)
            m_lBurstCount = lAmmo;

        if (m_lBurstCount != 0 && m_bBulletInChamber)
            EnableClassCall(16);
        else
            EmptyFire();
    }

    // vtbl slot 200  PC 0x50FBE0
    void ZItemWeapon::FireEnd()
    {
        ExGeomData* pExData = m_pExData;
        if (pExData == nullptr)
            return;

        ZGeomEventList& events = pExData->_Events;
        if (!events.ChkEvents())
            return;

        // PC bails out on the (-2) sentinel ExGeomData pointer used for detached geoms.
        if (reinterpret_cast<std::intptr_t>(pExData) == static_cast<std::intptr_t>(-2))
            return;

        ZGeomEventListBuffers::ValueRun run {};
        events.InitValueRun(run);

        uint32_t lEvent = events.GetValueFromValueRun(run);
        while (!run.m_bFin)
        {
            ZASSERT(ZEventBuffer::m_Instance != nullptr);

            ZEventBase* pEvent = ZEventBuffer::m_Instance->ConvEventRefToPtr(lEvent);
            if (pEvent != nullptr && pEvent->m_pBaseGeom == this && (pEvent->m_lRoutCases & 0x10) != 0)
            {
                m_bRequestFireRelease = true;
                break;
            }

            events.NextValueRun(run);
            lEvent = events.GetValueFromValueRun(run);
        }
    }

    // vtbl slot 201  PC 0x5142F0
    void ZItemWeapon::FireRound()
    {
        if (g_lBlockFire)
            return;

        ZItemTemplateAmmo* pAmmo = GetAmmoTemplate();
        ZASSERT(pAmmo != nullptr);
        if (pAmmo == nullptr)
            return;

        ZGEOM* pOwner = GetItemOwner();
        if (pOwner == nullptr)
            return;

        const bool bInHand = (pOwner->BaseGeom()->m_lControl & 0x1000) != 0;

        ZMat3x3 mat;
        ZVector3 pos;

        if (bInHand)
        {
            GetItemRootTM(reinterpret_cast<float*>(&mat), reinterpret_cast<float*>(&pos));
        }
        else
        {
            mreset(mat.Get());
            vreset(pos.Get());

            GetMainMatPos(reinterpret_cast<float*>(&mat), reinterpret_cast<float*>(&pos), 25);
            pos.y += 125.0f;
            pos.z -= 200.0f;

            pOwner->GetRootMatPos(mat, pos);
        }

        const int lProjectilesPerShot = pAmmo->GetProjectilesPerShot();
        ZItemTemplateWeapon* pWeaponTemplate = GetWeaponTemplate();
        ZGEOM* pProjectileAlign = pWeaponTemplate->m_pProjectileAlign;

        if (pProjectileAlign != nullptr)
        {
            // In hand the muzzle position/matrix come from ZItemTemplateWeapon::m_pProjectileAlign,
            // otherwise the projectile starts at the muzzle-less root point computed above.
            ZVector3 vProjectilePos = pos;

            if (bInHand)
            {
                ZMat3x3 alignMat;
                ZVector3 alignPos;
                pProjectileAlign->GetMatPos(alignMat, alignPos);

                mmmul(alignMat.data, mat.data);
                TransformRootVector(alignPos, mat);
                vadd(alignPos.Get(), pos.Get());

                vProjectilePos = alignPos;
            }

            // CProjectileActivate (PC ctor 0x510BB0) carries owner/weapon refs, muzzle velocity,
            // far range, explode-on-impact (WT_MOLOTOV == 12), can-penetrate, the weapon target,
            // precision, the ammo material id and the weapon template. The PC ctor leaves the
            // canPenetrateBody/sendUnderFireWarning flags unwritten, so the aggregate is
            // zero-initialised here.
            CProjectileActivate activate{};
            activate.m_rOwner = pOwner->GetRef();
            activate.m_rWeapon = GetRef();
            activate.m_fSpeed = pWeaponTemplate->GetMuzzleVelocity();
            activate.m_fRange = pWeaponTemplate->GetFarRange();
            activate.m_bExplodeOnImpact = pWeaponTemplate->GetWeaponType() == 12;
            activate.m_bCanPenetrate = pAmmo->GetCanPenetrate();
            activate.m_vTarget = *static_cast<const ZVector3*>(Target());
            activate.m_fPrecision = pWeaponTemplate->GetPrecisionDegrees();

            int lMaterialEnumId = 0;
            pAmmo->GetMaterialEnumId(&lMaterialEnumId);
            activate.m_eAmmoMaterialEnumId = static_cast<uint32_t>(lMaterialEnumId);

            activate.m_pWeaponTemplate = pWeaponTemplate;

            for (int i = 0; i < lProjectilesPerShot; ++i)
            {
                ZGEOM* pProjectile = pAmmo->GetProjectileInstance();
                if (pProjectile == nullptr)
                    continue;

                // When the owner is the player aiming through a scope, the projectile starts at
                // the camera root point instead of the muzzle.
                if ((ZPlayer::m_Mask & pOwner->GetObjectId()) == ZPlayer::m_Id)
                {
                    auto* pHitman = static_cast<Hitman::ZHitman3*>(pOwner);
                    if (pHitman->IsInScopeMode())
                    {
                        vProjectilePos = ZVector3(0.0f, 0.0f, 0.0f);
                        pHitman->m_pCameraControl->GetGeom()->GetRootPoint(vProjectilePos);
                    }
                }

                pProjectile->SetPos(vProjectilePos);
                pProjectile->SendCommand(g_MSG_Activate, &activate, nullptr);
                OnActivateProjectile(pProjectile);
            }
        }

        // When the weapon template's muzzle effect is a ZAllocMany, the PC clones one entry from
        // it at m_pMuzzleFlashAlign into the item owner's room (ZAllocMany::DuplicateInit, vtable
        // PC 0x4FFC80) instead of spawning the particle-controller effects, and skips the muzzle
        // light below.
        auto* pMuzzleEffect = static_cast<ZAllocMany*>(pWeaponTemplate->GetMuzzleEffect());
        const bool bHasMuzzleAlloc = pMuzzleEffect != nullptr
            && (ZAllocMany::m_Mask & pMuzzleEffect->GetObjectId()) == ZAllocMany::m_Id;

        if (bInHand)
        {
            if (bHasMuzzleAlloc)
            {
                if (ZGEOM* pMuzzleFlashAlign = pWeaponTemplate->m_pMuzzleFlashAlign)
                {
                    ZMat3x3 flashMat;
                    ZVector3 flashPos;
                    pMuzzleFlashAlign->GetMatPos(flashMat, flashPos);
                    TransformRootVector(flashPos, mat);
                    vadd(flashPos.Get(), pos.Get());
                    mmmul(flashMat.data, mat.data);

                    ZROOM* pOwnerRoom = pOwner->BaseGeom()->GetOwnerRoom();
                    pOwnerRoom->GetLocalMatPos(flashMat, flashPos);
                    pMuzzleEffect->DuplicateInit(pOwnerRoom, &flashMat, &flashPos, nullptr, true);
                }
            }
            else if (auto* pParticleController = static_cast<ZParticleController*>(ZGEOM::RefToPtr(m_rParticleController)))
            {
                // PC 0x5142F0: all particle-controller muzzle effects spawn inside the in-hand
                // branch through the ZParticleController referenced by m_rParticleController
                // (CreateParticle 0x4EFAC0).
                const TIMETYPE fireTime = g_pSysInterface->FrameTime;

                // Muzzle smoke: +-15 randomised offsets around m_pMuzzleSmokeAlign and an
                // FRand-scaled direction.
                if (m_lMuzzleSmokeIndex != 0 && pWeaponTemplate->m_pMuzzleSmokeAlign != nullptr)
                {
                    ZMat3x3 smokeMat;
                    ZVector3 smokePos;
                    pWeaponTemplate->m_pMuzzleSmokeAlign->GetMatPos(smokeMat, smokePos);
                    mmmul(smokeMat.data, mat.data);
                    TransformRootVector(smokePos, mat);
                    vadd(smokePos.Get(), pos.Get());

                    smokePos.y += g_pSysInterface->FRand(const_cast<char*>(__FILE__), __LINE__) * 30.0f - 15.0f;
                    smokePos.z += g_pSysInterface->FRand(const_cast<char*>(__FILE__), __LINE__) * 30.0f - 15.0f;

                    ZVector3 smokeDir;
                    pWeaponTemplate->m_pMuzzleSmokeAlign->GetRootVect(smokeDir);
                    vscalar(smokeDir.Get(), (1.0f - g_pSysInterface->FRand(const_cast<char*>(__FILE__), __LINE__) * 0.5f) * -1000.0f);

                    pParticleController->CreateParticle(m_lMuzzleSmokeIndex, smokePos.Get(), smokeDir.Get(), fireTime);
                }

                // Muzzle fire: five spawns starting at m_pMuzzleFlashAlign, spread +8 each step.
                if (m_lMuzzleFireIndex != 0 && pWeaponTemplate->m_pMuzzleFlashAlign != nullptr)
                {
                    ZMat3x3 flashMat;
                    ZVector3 flashPos;
                    pWeaponTemplate->m_pMuzzleFlashAlign->GetMatPos(flashMat, flashPos);
                    mmmul(flashMat.data, mat.data);

                    ZVector3 fireDir;
                    vreset(fireDir.Get());

                    for (int i = 0; i < 5; ++i)
                    {
                        ZVector3 firePos;
                        vmmul(firePos.Get(), flashPos.Get(), flashMat.data);
                        vadd(firePos.Get(), pos.Get());

                        pParticleController->CreateParticle(m_lMuzzleFireIndex, firePos.Get(), fireDir.Get(), fireTime);

                        vaddscalar(flashPos.Get(), flashPos.Get(), &flashMat.data[6], 8.0f);
                    }
                }

                // Cartridge: direction taken from m_pCartridgeAlign and scaled by the weapon
                // template's cartridge speed.
                if (m_lCartridgeIndex != 0 && pWeaponTemplate->m_pCartridgeAlign != nullptr)
                {
                    ZMat3x3 cartMat;
                    ZVector3 cartPos;
                    pWeaponTemplate->m_pCartridgeAlign->GetMatPos(cartMat, cartPos);
                    mmmul(cartMat.data, mat.data);
                    TransformRootVector(cartPos, mat);
                    vadd(cartPos.Get(), pos.Get());

                    ZVector3 cartDir(cartMat.data[6], cartMat.data[7], cartMat.data[8]);
                    vscalar(cartDir.Get(), pWeaponTemplate->GetCartridgeSpeed()
                        * (1.0f - g_pSysInterface->FRand(const_cast<char*>(__FILE__), __LINE__) * 0.1f));

                    pParticleController->CreateParticle(m_lCartridgeIndex, cartPos.Get(), cartDir.Get(), fireTime);
                }
            }
        }

        if (!bHasMuzzleAlloc)
        {
            if (ZGEOM* pMuzzleLight = ZGEOM::RefToPtr(m_rMuzzleLight))
            {
                ZVector3 vMuzzleLightPos;
                vmmul(vMuzzleLightPos.Get(), m_vMuzzleLightAlign.Get(), mat.Get());
                vadd(vMuzzleLightPos.Get(), pos.Get());

                pMuzzleLight->SetRootTM(mat, vMuzzleLightPos);

                if ((pMuzzleLight->BaseGeom()->m_lControl & 0x400) != 0)
                    pMuzzleLight->MakeActive();
            }
        }

        CopyExData(this);

        if (ZGEOM* pOwnerMsg = GetItemOwner())
            pOwnerMsg->SendCommand(g_MSG_WeaponFired, nullptr, nullptr);
    }

    // vtbl slot 202  PC 0x4715C0 (shared no-op stub)
    void ZItemWeapon::EmptyFire()
    {
    }

    // vtbl slot 203  PC 0x50FD00
    void ZItemWeapon::FireRoundActivateAnimation()
    {
        ZGEOM* pSlide = ZGEOM::RefToPtr(m_rSlide);
        if (pSlide == nullptr)
            return;

        int lValue = 0;
        pSlide->SendCommand(ZMSG_ANIM_2058, &lValue, nullptr);

        lValue = (GetProjectilesInMagazine() == 1) ? 1 : 3;
        pSlide->SendCommand(ZMSG_ANIM_2059, &lValue, nullptr);

        pSlide->SendCommand(g_MSG_Activate, this, nullptr);
    }

    // vtbl slot 204  PC 0x6F7550 (nullsub)
    void ZItemWeapon::OnActivateProjectile(ZGEOM* /*pProjectile*/)
    {
    }

    // vtbl slot 205  PC 0x50F950
    void ZItemWeapon::CopyGeom(ZGEOM* from, ZGEOM* /*unused*/, ZGROUP* inGroup, bool makeActive)
    {
        ZASSERT(from != nullptr);
        ZASSERT(inGroup != nullptr);

        ZGEOM* pCopy = from->DuplicateToResource(inGroup, 'ITEM', nullptr, true);
        if (pCopy == nullptr)
            return;

        if (makeActive)
            pCopy->MakeActive();

        ZEventBase* pEvent = pCopy->FindEvent("ItemState*");
        if (pEvent != nullptr)
            ZASSERT(pEvent->m_pBaseGeom == pCopy);
        if (pEvent != nullptr)
            pEvent->Delete();

        ZMat3x3 mat;
        ZVector3 pos;
        inGroup->GetMatPos(mat, pos);
        pCopy->SetMatPos(mat, pos);
        pCopy->DoInit();
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        // WEAPONOPERATION: also declared as an enum property for m_eWeaponOperation (bitfield enum in
        // the template). Mirrors ZItemTemplateWeapon.cpp's declaration.
        static ZEnumEntry WeaponOperationEntries[] = {
            {nullptr, static_cast<int>(EWeaponOperation::WO_MANUAL), "WO_MANUAL"},
            {&WeaponOperationEntries[0], static_cast<int>(EWeaponOperation::WO_SEMIAUTO), "WO_SEMIAUTO"},
            {&WeaponOperationEntries[1], static_cast<int>(EWeaponOperation::WO_FULLAUTO), "WO_FULLAUTO"},
            {&WeaponOperationEntries[2], static_cast<int>(EWeaponOperation::WO_FULLAUTO3), "WO_FULLAUTO3"},
            {&WeaponOperationEntries[3], 0x7FFFFFFF, "WO_FORCE32"}};
        [[maybe_unused]] static ZEnumInfo WeaponOperationInfo{&WeaponOperationEntries[4], "WEAPONOPERATION", sizeof(WEAPONOPERATION)};

        // PC chain (Info.First = 0x0080C7D4): m_vTarget -> m_rAmmoTemplate -> m_lProjectilesInMagazine
        // -> m_bBulletInChamber -> m_lBurstCount -> m_bRequestFireRelease -> m_bReloading
        // -> m_bChambering -> m_bTriggerHeld -> m_eWeaponOperation -> m_prtWeaponParts
        // -> m_rMuzzleLight -> m_rSlide -> m_rClip -> m_bUseBulletsFromMagazine -> m_bWantSoundEvent.
        // Declared tail-first; the head (m_vTarget) is passed to DECLARE_GEOM_CLASS_IMPL.

        static RTP::ZDataProperty<bool> WantSoundEvent{
            .m_Node = {.m_Next = nullptr, .m_Name = "m_bWantSoundEvent", .m_Filter = 2},
            .m_VirtualTable = &RTP::VirtualTables::Data_bool,
            .m_Offset = CLASS_PROPERTY(ZItemWeapon, m_bWantSoundEvent)};

        static RTP::ZDataProperty<bool> UseBulletsFromMagazine{
            .m_Node = {.m_Next = WantSoundEvent, .m_Name = "m_bUseBulletsFromMagazine", .m_Filter = 2},
            .m_VirtualTable = &RTP::VirtualTables::Data_bool,
            .m_Offset = CLASS_PROPERTY(ZItemWeapon, m_useBulletsFromMagazine)};

        static RTP::ZDataProperty<ZGEOMREF> Clip{
            .m_Node = {.m_Next = UseBulletsFromMagazine, .m_Name = "m_rClip", .m_Filter = 2},
            .m_VirtualTable = &RTP::VirtualTables::Data_ZGEOMREF,
            .m_Offset = reinterpret_cast<ZGEOMREF*>(CLASS_PROPERTY(ZItemWeapon, m_rClip))};

        static RTP::ZDataProperty<ZGEOMREF> Slide{
            .m_Node = {.m_Next = Clip, .m_Name = "m_rSlide", .m_Filter = 2},
            .m_VirtualTable = &RTP::VirtualTables::Data_ZGEOMREF,
            .m_Offset = reinterpret_cast<ZGEOMREF*>(CLASS_PROPERTY(ZItemWeapon, m_rSlide))};

        static RTP::ZDataProperty<ZGEOMREF> MuzzleLight{
            .m_Node = {.m_Next = Slide, .m_Name = "m_rMuzzleLight", .m_Filter = 2},
            .m_VirtualTable = &RTP::VirtualTables::Data_ZGEOMREF,
            .m_Offset = reinterpret_cast<ZGEOMREF*>(CLASS_PROPERTY(ZItemWeapon, m_rMuzzleLight))};

        static RTP::ZDataProperty<REFTAB*> WeaponParts{
            .m_Node = {.m_Next = MuzzleLight, .m_Name = "m_prtWeaponParts", .m_Filter = 2},
            .m_VirtualTable = &RTP::VirtualTables::Data_REFTAB_ptr,
            .m_Offset = CLASS_PROPERTY(ZItemWeapon, m_prtWeaponParts)};

        static RTP::ZEnumProperty WeaponOperation{
            .m_Node = {.m_Next = WeaponParts, .m_Name = "m_eWeaponOperation", .m_Filter = 2},
            .m_VirtualTable = &RTP::VirtualTables::Enum,
            .m_Offset = CLASS_PROPERTY(ZItemWeapon, m_eWeaponOperation),
            .m_Info = &WeaponOperationInfo};

        static RTP::ZDataProperty<bool> TriggerHeld{
            .m_Node = {.m_Next = WeaponOperation, .m_Name = "m_bTriggerHeld", .m_Filter = 2},
            .m_VirtualTable = &RTP::VirtualTables::Data_bool,
            .m_Offset = CLASS_PROPERTY(ZItemWeapon, m_bTriggerHeld)};

        static RTP::ZDataProperty<bool> Chambering{
            .m_Node = {.m_Next = TriggerHeld, .m_Name = "m_bChambering", .m_Filter = 2},
            .m_VirtualTable = &RTP::VirtualTables::Data_bool,
            .m_Offset = CLASS_PROPERTY(ZItemWeapon, m_bChambering)};

        static RTP::ZDataProperty<bool> Reloading{
            .m_Node = {.m_Next = Chambering, .m_Name = "m_bReloading", .m_Filter = 2},
            .m_VirtualTable = &RTP::VirtualTables::Data_bool,
            .m_Offset = CLASS_PROPERTY(ZItemWeapon, m_bReloading)};

        static RTP::ZDataProperty<bool> RequestFireRelease{
            .m_Node = {.m_Next = Reloading, .m_Name = "m_bRequestFireRelease", .m_Filter = 2},
            .m_VirtualTable = &RTP::VirtualTables::Data_bool,
            .m_Offset = CLASS_PROPERTY(ZItemWeapon, m_bRequestFireRelease)};

        static RTP::ZDataProperty<int> BurstCount{
            .m_Node = {.m_Next = RequestFireRelease, .m_Name = "m_lBurstCount", .m_Filter = 2},
            .m_VirtualTable = &RTP::VirtualTables::Data_int,
            .m_Offset = CLASS_PROPERTY(ZItemWeapon, m_lBurstCount)};

        static RTP::ZDataProperty<bool> BulletInChamber{
            .m_Node = {.m_Next = BurstCount, .m_Name = "m_bBulletInChamber", .m_Filter = 2},
            .m_VirtualTable = &RTP::VirtualTables::Data_bool,
            .m_Offset = CLASS_PROPERTY(ZItemWeapon, m_bBulletInChamber)};

        // This is the only property with m_Filter = 3 in the PC chain.
        static RTP::ZDataProperty<int> ProjectilesInMagazine{
            .m_Node = {.m_Next = BulletInChamber, .m_Name = "m_lProjectilesInMagazine", .m_Filter = 3},
            .m_VirtualTable = &RTP::VirtualTables::Data_int,
            .m_Offset = CLASS_PROPERTY(ZItemWeapon, m_lProjectilesInMagazine)};

        static RTP::ZDataProperty<ZGEOMREF> AmmoTemplate{
            .m_Node = {.m_Next = ProjectilesInMagazine, .m_Name = "m_rAmmoTemplate", .m_Filter = 2},
            .m_VirtualTable = &RTP::VirtualTables::Data_ZGEOMREF,
            .m_Offset = reinterpret_cast<ZGEOMREF*>(CLASS_PROPERTY(ZItemWeapon, m_rAmmoTemplate))};

        static RTP::ZDataProperty<float[3]> Target{
            .m_Node = {.m_Next = AmmoTemplate, .m_Name = "m_vTarget", .m_Filter = 2},
            .m_VirtualTable = &RTP::VirtualTables::Data_float_3,
            .m_Offset = reinterpret_cast<float(*)[3]>(CLASS_PROPERTY(ZItemWeapon, m_vTarget))};
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZItemWeapon,          // ClassName
        ZItem,                // BaseClass
        0x0099C168,           // OldClassInfoAddr (ZItemWeapon::m_OldClassInfo)
        "ZItemWeapon",        // FactoryName
        0x00773EE8,           // FactoryNameAddr
        cProperties::Target,  // FirstProperty (PC 0x0080C7D4)
        0x0080C7E8,           // PropertiesAddr (ZItemWeapon::Info)
        0x0099BF58,           // IdAddr (ZItemWeapon::m_Id)
        0x0099BF5C            // MaskAddr (ZItemWeapon::m_Mask)
    );
#   pragma endregion
}
