#include <Glacier/GameBase/Action/ZActionReloadItem.h>

#include <Glacier/Animation/Header.h>
#include <Glacier/Animation/Manager.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/IK/ZIKHAND.h>
#include <Glacier/IK/ZLNKWHANDS.h>
#include <Glacier/Items/ZItemTemplateWeapon.h>
#include <Glacier/Items/ZItemWeapon.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZUniAssert.h>

#include <algorithm>
#include <cstring>

namespace Glacier
{
    ZActionReloadItem::ZActionReloadItem(uint32_t actionId, ZLNKWHANDS* pActor, ZIKHAND* pHand,
        Animation::Header* pAnimReload, bool bMirrorAnim)
        : ZLnkAction(actionId),
          m_pActor(pActor),
          m_pHand(pHand),
          m_pAnimReloadRevolver(nullptr),
          m_pAnimReloadPistol(nullptr),
          m_pAnimReloadSubMachineGun(nullptr),
          m_pAnimReloadRifle(nullptr),
          m_pAnimReloadPumpgun(nullptr),
          m_pAnimReloadShotgun(nullptr),
          m_pAnimReloadRPG(nullptr),
          m_pCurrentReloadAnim(pAnimReload),
          m_fReloadStartTime(0.f),
          m_fCycleTime(0.f),
          m_fSingleLoadTimeStart(0.f),
          m_fSingleLoadTimeEnd(0.f),
          m_bMirrorAnim(bMirrorAnim),
          m_bFinishInNextCycle(false),
          m_bFinishCycle(false)
    {
        ZASSERT(m_pHand);
        ZASSERT(m_pActor);
    }

    ZActionReloadItem::ZActionReloadItem(uint32_t actionId, ZLNKWHANDS* pActor, ZIKHAND* pHand,
        Animation::Header* pAnimReloadRevolver, Animation::Header* pAnimReloadPistol,
        Animation::Header* pAnimReloadSubMachineGun, Animation::Header* pAnimReloadRifle,
        Animation::Header* pAnimReloadShotgun, Animation::Header* pAnimReloadPumpgun,
        Animation::Header* pAnimReloadRPG)
        : ZLnkAction(actionId),
          m_pActor(pActor),
          m_pHand(pHand),
          m_pAnimReloadRevolver(pAnimReloadRevolver),
          m_pAnimReloadPistol(pAnimReloadPistol),
          m_pAnimReloadSubMachineGun(pAnimReloadSubMachineGun),
          m_pAnimReloadRifle(pAnimReloadRifle),
          m_pAnimReloadPumpgun(pAnimReloadPumpgun),
          m_pAnimReloadShotgun(pAnimReloadShotgun),
          m_pAnimReloadRPG(pAnimReloadRPG),
          m_pCurrentReloadAnim(nullptr),
          m_fReloadStartTime(0.f),
          m_fCycleTime(0.f),
          m_fSingleLoadTimeStart(0.f),
          m_fSingleLoadTimeEnd(0.f),
          m_bMirrorAnim(false),
          m_bFinishInNextCycle(false),
          m_bFinishCycle(false)
    {
        ZASSERT(m_pHand);
        ZASSERT(m_pActor);
    }

    ZActionReloadItem::~ZActionReloadItem() = default;

    bool ZActionReloadItem::Execute()
    {
        auto* pWeapon = static_cast<ZItemWeapon*>(m_pHand->GetItem());
        ZASSERT(pWeapon);

        pWeapon->SetState(ITEMSTATE::eIS_NORMAL, nullptr);
        m_fReloadStartTime = static_cast<float>(g_pSysInterface->FrameTime);
        m_fCycleTime = 0.f;
        if (m_pCurrentReloadAnim)
            m_fSingleLoadTimeStart = 0.f;
        else
            m_pCurrentReloadAnim = GetReloadAnim(static_cast<ZItemTemplateWeapon*>(pWeapon->GetItemTemplate()));

        m_pActor->ClassCommand(g_pEngineData->RegisterZMsg("Reload", 0, __FILE__, __LINE__), nullptr);
        m_bFinishInNextCycle = false;
        m_bFinishCycle = false;
        SetSingleLoadStartEnd();
        return !WeaponReloaded(pWeapon);
    }

    void ZActionReloadItem::Terminate()
    {
        m_pActor->ClassCommand(g_pEngineData->RegisterZMsg("ReloadDone", 0, __FILE__, __LINE__), nullptr);
        if (auto* pWeapon = static_cast<ZItemWeapon*>(m_pHand->GetItem()))
            pWeapon->SetState(ITEMSTATE::eIS_NORMAL, nullptr);
    }

    bool ZActionReloadItem::Update()
    {
        auto* pWeapon = static_cast<ZItemWeapon*>(m_pHand->GetItem());
        if (!pWeapon)
            return false;

        if (!m_pCurrentReloadAnim)
        {
            pWeapon->SetState(ITEMSTATE::IS_EXTRA1, nullptr);
            ReloadWeapon(pWeapon, 0);
            return false;
        }

        const float currentTime = static_cast<float>(g_pSysInterface->FrameTime);
        if (currentTime < m_fReloadStartTime)
            return true;

        pWeapon->SetState(ITEMSTATE::IS_EXTRA1, nullptr);
        const int frameCount = m_pCurrentReloadAnim->m_Frames - 1;
        float frame = 0.f;
        if (frameCount > 0)
        {
            if (m_fSingleLoadTimeStart == 0.f)
            {
                frame = (currentTime - m_fReloadStartTime) / (frameCount * Animation::Header::TIME_SCALE);
                if (frame > 1.f)
                {
                    ReloadWeapon(pWeapon, 0);
                    return false;
                }
            }
            else if (currentTime - m_fReloadStartTime >= m_fSingleLoadTimeStart)
            {
                m_fCycleTime += g_pSysInterface->DeltaFrameTime;
                if (WeaponReloaded(pWeapon))
                {
                    frame = (m_fCycleTime + m_fSingleLoadTimeEnd) / (frameCount * Animation::Header::TIME_SCALE);
                    if (frame > 1.f)
                    {
                        pWeapon->SetState(ITEMSTATE::eIS_NORMAL, nullptr);
                        return false;
                    }
                }
                else if (m_fSingleLoadTimeEnd - m_fSingleLoadTimeStart >= m_fCycleTime)
                {
                    frame = (m_fCycleTime + m_fSingleLoadTimeStart) / (frameCount * Animation::Header::TIME_SCALE);
                }
                else
                {
                    ReloadWeapon(pWeapon, 1);
                    frame = (m_fCycleTime + m_fSingleLoadTimeStart) / (frameCount * Animation::Header::TIME_SCALE);
                    m_fCycleTime = 0.f;
                }
            }
            else
            {
                frame = (currentTime - m_fReloadStartTime) / (frameCount * Animation::Header::TIME_SCALE);
            }
        }

        m_pActor->SetBoneFrameBlend(m_pCurrentReloadAnim, frame, m_pCurrentReloadAnim->GetBlendTime(), false,
            m_bMirrorAnim ? 0xC100 : 0x4100);
        return true;
    }

    const char* ZActionReloadItem::Name() const
    {
        return "ZActionReloadItem";
    }

    Animation::Header* ZActionReloadItem::GetReloadAnim(ZItemTemplateWeapon* pWeaponTemplate)
    {
        m_fSingleLoadTimeStart = 0.f;
        switch (pWeaponTemplate->GetWeaponType())
        {
        case WT_PISTOL: return m_pAnimReloadPistol;
        case WT_REVOLVER: return m_pAnimReloadRevolver;
        case WT_SUBMACHINEGUN: return m_pAnimReloadSubMachineGun;
        case WT_MACHINEGUN:
        case WT_RIFLE: return m_pAnimReloadRifle ? m_pAnimReloadRifle : m_pAnimReloadRevolver;
        case WT_PUMPGUN:
            m_fSingleLoadTimeStart = 0.44f;
            m_fSingleLoadTimeEnd = 0.68f;
            return m_pAnimReloadPumpgun ? m_pAnimReloadPumpgun : m_pAnimReloadRevolver;
        case WT_SHOTGUN: return m_pAnimReloadShotgun ? m_pAnimReloadShotgun : m_pAnimReloadRevolver;
        case WT_ROCKETLAUNCHER: return m_pAnimReloadRPG ? m_pAnimReloadRPG : m_pAnimReloadRevolver;
        case WT_GRENADE:
        case WT_MOLOTOV: return nullptr;
        default: return m_pAnimReloadRevolver;
        }
    }

    uint32_t ZActionReloadItem::GetReloadAnimId(Animation::Header* pAnimation)
    {
        if (pAnimation == m_pAnimReloadRevolver) return 1;
        if (pAnimation == m_pAnimReloadPistol) return 2;
        if (pAnimation == m_pAnimReloadSubMachineGun) return 3;
        if (pAnimation == m_pAnimReloadRifle) return 4;
        if (pAnimation == m_pAnimReloadShotgun) return 5;
        if (pAnimation == m_pAnimReloadPumpgun) return 6;
        ZASSERT(pAnimation == m_pAnimReloadRPG);
        return 7;
    }

    Animation::Header* ZActionReloadItem::GetReloadAnimFromId(uint32_t animationId)
    {
        switch (animationId)
        {
        case 1: return m_pAnimReloadRevolver;
        case 2: return m_pAnimReloadPistol;
        case 3: return m_pAnimReloadSubMachineGun;
        case 4: return m_pAnimReloadRifle;
        case 5: return m_pAnimReloadShotgun;
        case 6: return m_pAnimReloadPumpgun;
        case 7: return m_pAnimReloadRPG;
        default: ZASSERT(false); return nullptr;
        }
    }

    float ZActionReloadItem::GetStartTime() const { return m_fReloadStartTime; }
    ZIKHAND* ZActionReloadItem::GetHand() const { return m_pHand; }
    ZLNKWHANDS* ZActionReloadItem::GetActor() const { return m_pActor; }

    bool ZActionReloadItem::SupportsLoadSave()
    {
        return m_lActionId == 13 && !strcmp(Name(), "ZActionReloadItem");
    }

    void ZActionReloadItem::LoadSave(ISerializerStream& stream, bool bSaving)
    {
        ZLnkAction::LoadSave(stream, bSaving);
        stream.Exchange("pGeom", m_pActor);
        int32_t handId = m_pHand == &m_pActor->m_LHand ? 1 : 2;
        stream.Exchange("handID", handId);
        m_pHand = handId == 1 ? &m_pActor->m_LHand : &m_pActor->m_RHand;
        stream.Exchange("m_pAnimReloadRevolver", m_pAnimReloadRevolver);
        stream.Exchange("m_pAnimReloadPistol", m_pAnimReloadPistol);
        stream.Exchange("m_pAnimReloadSubMachineGun", m_pAnimReloadSubMachineGun);
        stream.Exchange("m_pAnimReloadRifle", m_pAnimReloadRifle);
        stream.Exchange("m_pAnimReloadPumpgun", m_pAnimReloadPumpgun);
        stream.Exchange("m_pAnimReloadShotgun", m_pAnimReloadShotgun);
        stream.Exchange("m_pAnimReloadRPG", m_pAnimReloadRPG);
        stream.Exchange("m_pCurrentReloadAnim", m_pCurrentReloadAnim);
        stream.Exchange("m_fReloadStartTime", m_fReloadStartTime);
        stream.Exchange("m_fCycleTime", m_fCycleTime);
        stream.Exchange("m_fSingleLoadTimeStart", m_fSingleLoadTimeStart);
        stream.Exchange("m_fSingleLoadTimeEnd", m_fSingleLoadTimeEnd);
        stream.Exchange("m_bMirrorAnim", m_bMirrorAnim);
        stream.Exchange("m_bFinishInNextCycle", m_bFinishInNextCycle);
        stream.Exchange("m_bFinishCycle", m_bFinishCycle);
    }

    bool ZActionReloadItem::WeaponReloaded(ZItemWeapon* pWeapon) const
    {
        return !m_pActor->GetAmmoFor(pWeapon)
            || pWeapon->GetProjectilesPerMagazine() == pWeapon->GetProjectilesInMagazine();
    }

    void ZActionReloadItem::ReloadWeapon(ZItemWeapon* pWeapon, int ammoCount)
    {
        ZASSERT(pWeapon);
        const int available = m_pActor->GetAmmoFor(pWeapon);
        const int loaded = pWeapon->GetProjectilesInMagazine();
        const int capacity = pWeapon->GetProjectilesPerMagazine();
        int transferred = std::min(available, capacity - loaded);
        if (ammoCount && ammoCount < transferred)
            transferred = ammoCount;

        m_pActor->SetAmmoFor(pWeapon, available - transferred);
        pWeapon->SetProjectilesInMagazine(loaded + transferred);
        pWeapon->SetBulletInChamber(true);
        if (m_fSingleLoadTimeStart == 0.f && m_fSingleLoadTimeEnd == 0.f)
            pWeapon->SetState(ITEMSTATE::eIS_NORMAL, nullptr);
    }

    void ZActionReloadItem::SetSingleLoadStartEnd()
    {
        m_fSingleLoadTimeStart = 0.f;
        m_fSingleLoadTimeEnd = 0.f;
        if (!m_pCurrentReloadAnim || !Animation::instance)
            return;

        int32_t index = 0;
        const int32_t startFrame = Animation::instance->GetFrameFromMetaValue(
            m_pCurrentReloadAnim->m_MetaDataOffset, 1, &index);
        index = 0;
        const int32_t endFrame = Animation::instance->GetFrameFromMetaValue(
            m_pCurrentReloadAnim->m_MetaDataOffset, 2, &index);
        if (startFrame != -1 && endFrame != -1)
        {
            m_fSingleLoadTimeStart = startFrame * Animation::Header::TIME_SCALE;
            m_fSingleLoadTimeEnd = endFrame * Animation::Header::TIME_SCALE;
        }
    }
}
