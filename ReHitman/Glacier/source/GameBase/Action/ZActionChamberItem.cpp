#include <Glacier/GameBase/Action/ZActionChamberItem.h>

#include <Glacier/Animation/Header.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/IK/ZIKHAND.h>
#include <Glacier/IK/ZLNKWHANDS.h>
#include <Glacier/Items/ZItemTemplateWeapon.h>
#include <Glacier/Items/ZItemWeapon.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZUniAssert.h>

#include <cstring>

namespace Glacier
{
    ZActionChamberItem::ZActionChamberItem(uint32_t actionId, ZLNKWHANDS* pActor, ZIKHAND* pHand,
        Animation::Header* pAnimChamberGun, Animation::Header* pAnimChamberRifle,
        Animation::Header* pAnimChamberShotgun, Animation::Header* pAnimChamberPumpgun,
        Animation::Header* pAnimChamberRPG, Animation::Header* pAnimChamberGrenade)
        : ZLnkAction(actionId),
          m_pActor(pActor),
          m_pHand(pHand),
          m_pAnimChamberGun(pAnimChamberGun),
          m_pAnimChamberRifle(pAnimChamberRifle),
          m_pAnimChamberPumpgun(pAnimChamberPumpgun),
          m_pAnimChamberShotgun(pAnimChamberShotgun),
          m_pAnimChamberRPG(pAnimChamberRPG),
          m_pAnimChamberGrenade(pAnimChamberGrenade),
          m_pCurrentChamberAnim(nullptr),
          m_fChamberStartTime(0.f)
    {
        ZASSERT(m_pHand);
        ZASSERT(m_pActor);
    }

    ZActionChamberItem::~ZActionChamberItem() = default;

    bool ZActionChamberItem::Execute()
    {
        auto* pWeapon = static_cast<ZItemWeapon*>(m_pHand->GetItem());
        ZASSERT(pWeapon);
        m_fChamberStartTime = static_cast<float>(g_pSysInterface->FrameTime);
        m_pCurrentChamberAnim = GetChamberAnim(static_cast<ZItemTemplateWeapon*>(pWeapon->GetItemTemplate()));
        return true;
    }

    bool ZActionChamberItem::Update()
    {
        auto* pWeapon = static_cast<ZItemWeapon*>(m_pHand->GetItem());
        if (!pWeapon)
            return false;

        if (!m_pCurrentChamberAnim)
        {
            pWeapon->SetState(ITEMSTATE::IS_EXTRA2, nullptr);
            ChamberWeapon(pWeapon);
            return false;
        }

        const float currentTime = static_cast<float>(g_pSysInterface->FrameTime);
        if (currentTime < m_fChamberStartTime)
            return true;

        pWeapon->SetState(ITEMSTATE::IS_EXTRA2, nullptr);
        const int frameCount = m_pCurrentChamberAnim->m_Frames - 1;
        float frame = 0.f;
        if (frameCount > 0)
        {
            frame = (currentTime - m_fChamberStartTime) / (frameCount * Animation::Header::TIME_SCALE);
            if (frame > 1.f)
            {
                ChamberWeapon(pWeapon);
                return false;
            }
        }

        m_pActor->SetBoneFrameBlend(m_pCurrentChamberAnim, frame, m_pCurrentChamberAnim->GetBlendTime(), false, 0x100);
        return true;
    }

    const char* ZActionChamberItem::Name() const
    {
        return "ZActionChamberItem";
    }

    bool ZActionChamberItem::SupportsLoadSave()
    {
        return (m_lActionId == 14 || m_lActionId == 15) && !strcmp(Name(), "ZActionChamberItem");
    }

    void ZActionChamberItem::LoadSave(ISerializerStream& stream, bool bSaving)
    {
        ZLnkAction::LoadSave(stream, bSaving);
        stream.Exchange("pActor", m_pActor);
        int32_t handId = m_pHand == &m_pActor->m_LHand ? 1 : 2;
        stream.Exchange("handID", handId);
        m_pHand = handId == 1 ? &m_pActor->m_LHand : &m_pActor->m_RHand;
        stream.Exchange("m_pAnimChamberGun", m_pAnimChamberGun);
        stream.Exchange("m_pAnimChamberRifle", m_pAnimChamberRifle);
        stream.Exchange("m_pAnimChamberPumpgun", m_pAnimChamberPumpgun);
        stream.Exchange("m_pAnimChamberShotgun", m_pAnimChamberShotgun);
        stream.Exchange("m_pAnimChamberRPG", m_pAnimChamberRPG);
        stream.Exchange("m_pAnimChamberGrenade", m_pAnimChamberGrenade);
        stream.Exchange("m_pCurrentChamberAnim", m_pCurrentChamberAnim);
        stream.Exchange("m_fChamberStartTime", m_fChamberStartTime);
    }

    Animation::Header* ZActionChamberItem::GetChamberAnim(ZItemTemplateWeapon* pWeaponTemplate)
    {
        switch (pWeaponTemplate->GetWeaponType())
        {
        case WT_PISTOL:
        case WT_REVOLVER:
        case WT_SUBMACHINEGUN: return m_pAnimChamberGun;
        case WT_MACHINEGUN:
        case WT_RIFLE: return m_pAnimChamberRifle;
        case WT_PUMPGUN: return m_pAnimChamberPumpgun;
        case WT_SHOTGUN: return m_pAnimChamberShotgun;
        case WT_ROCKETLAUNCHER: return m_pAnimChamberRPG;
        case WT_GRENADE: return m_pAnimChamberGrenade;
        default: return m_pAnimChamberGun;
        }
    }

    void ZActionChamberItem::ChamberWeapon(ZItemWeapon* pWeapon)
    {
        ZASSERT(pWeapon);
        pWeapon->SetBulletInChamber(true);
        pWeapon->SetState(ITEMSTATE::eIS_NORMAL, nullptr);
        m_pActor->ClassCommand(g_pEngineData->RegisterZMsg("ChamberDone", 0, __FILE__, __LINE__), nullptr);
    }
}
