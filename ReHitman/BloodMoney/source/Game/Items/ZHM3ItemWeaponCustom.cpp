#include <BloodMoney/Game/Items/ZHM3ItemWeaponCustom.h>

#include <BloodMoney/Game/Items/ZHM3ItemWeaponCustomTemplate.h>
#include <BloodMoney/Game/Items/ZHM3ItemTemplateAmmo.h>
#include <BloodMoney/Game/Items/ZHM3ItemTool.h>
#include <BloodMoney/Game/ZHM3GameData.h>
#include <BloodMoney/Game/ZHM3WeaponUpgradeControl.h>

#include <Glacier/Data/ZGameData.h>
#include <Glacier/IK/ZBoneModifyBase.h>
#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/Render/Prim/SBoneDefinition.h>
#include <Glacier/ZSTL/StringUtils.h>
#include <Glacier/ZUniAssert.h>


namespace Hitman
{
    // PC 0x64A880.
    void ZHM3ItemWeaponCustom::GetAnims()
    {
        if (m_pGround == nullptr)
            return;

        m_pAnimReloadBoltAction = m_pGround->GetAnimHeaderFromHandleName("Reload_Bolt");
        m_pAnimReloadDoubleCapMag = m_pGround->GetAnimHeaderFromHandleName("Reload_DoubleClip");
        m_pAnimReloadBeltFeeding = m_pGround->GetAnimHeaderFromHandleName("Reload_100shot");
    }

    // PC 0x64C770. The PC symbol is `ZHM3ItemWeaponCustom::SetMuzzleVelocity`; it forwards the value
    // scaled by 100 to the custom template. (Declared as GetMuzzleVelocity(float) in this header.)
    void* ZHM3ItemWeaponCustom::GetMuzzleVelocity(float fVelocity)
    {
        ZHM3ItemWeaponCustomTemplate* pTemplate = GetHM3WeaponTemplate();
        ZASSERT(pTemplate != nullptr);

        pTemplate->SetMuzzleVelocity(fVelocity * 100.0f);
        return nullptr;
    }

    // PC 0x64C820.
    void ZHM3ItemWeaponCustom::SetSilencerType(ESilencerType type)
    {
        ZHM3ItemWeaponCustomTemplate* pTemplate = GetHM3WeaponTemplate();
        ZASSERT(pTemplate != nullptr);

        pTemplate->m_CustomData.m_eSilencerType = type;
    }

    // PC 0x64F190.
    void ZHM3ItemWeaponCustom::HideBone(uint32_t lBoneId, bool bHide)
    {
        if (m_pGround == nullptr)
            return;

        const uint8_t lBoneNr = static_cast<uint8_t>(m_pGround->GetBoneNrFromId(static_cast<uint8_t>(lBoneId)));
        m_pGround->GetBoneModifier()->HideBone(m_pGround->BaseGeom(), lBoneNr, bHide);

        // For the custom hardballer pickup the bone is also toggled on the Ground01/Ground02 sub-geoms.
        if (ZHM3ItemTool::GetHM3Type(this) == EHM3ItemType::eHM3CustomHardballerPickup)
        {
            Glacier::ZLNKOBJ* pGround = static_cast<Glacier::ZLNKOBJ*>(FindGeom("Ground02", nullptr));
            if (pGround == nullptr || pGround == m_pGround)
                pGround = static_cast<Glacier::ZLNKOBJ*>(FindGeom("Ground01", nullptr));
            if (pGround != nullptr)
                pGround->HideBone(lBoneNr, bHide);
        }
    }

    // PC 0x64C480. Removes the bone id from the visible list (last entry back-fills the hole).
    void ZHM3ItemWeaponCustom::RemoveVisibleBone(uint32_t lBoneId)
    {
        const int32_t lIndex = m_VisibleBones.Find(&lBoneId);
        if (lIndex != -1)
            m_VisibleBones.Remove(static_cast<uint32_t>(lIndex));
    }

    // PC 0x64F260.
    void ZHM3ItemWeaponCustom::RemoveVisibleBone(const char* psBoneName)
    {
        RemoveVisibleBone(static_cast<uint32_t>(GetBoneIdFromBoneName(psBoneName)));
    }

    // PC 0x650450.
    void ZHM3ItemWeaponCustom::UpdateWeaponPartDrawStatus()
    {
        if (m_pGround == nullptr)
            return;

        ZASSERT((m_pGround->GetObjectId() & Glacier::ZLNKOBJ::m_Mask) == Glacier::ZLNKOBJ::m_Id);

        const Glacier::SBoneDefinition* pDefinitions = m_pGround->GetBoneDefinitions();
        ZASSERT(pDefinitions != nullptr);

        const uint32_t lActiveBones = m_pGround->NumActiveBones();
        for (uint32_t i = 1; i < lActiveBones; ++i)
        {
            const uint32_t lBoneId = pDefinitions[i].Id;
            HideBone(lBoneId, m_VisibleBones.Find(&lBoneId) == -1);
        }
    }

    // PC 0x6505E0.
    void ZHM3ItemWeaponCustom::ApplyUpgrades(char a1)
    {
        ZHM3ItemWeaponCustomTemplate* pTemplate = GetHM3WeaponTemplate();
        ZASSERT(pTemplate != nullptr);

        m_VisibleBones.Clear();

        ZHM3GameData* pGameData = static_cast<ZHM3GameData*>(Glacier::g_pGameData);
        if (pGameData != nullptr && pGameData->m_pWeaponUgradeControl != nullptr)
        {
            const EWeaponType eWeaponType = ZHM3WeaponUpgradeControl::GetWeaponType(pTemplate->Name());
            pGameData->m_pWeaponUgradeControl->ApplyDefaultUpgrades(eWeaponType, this);
        }

        pGameData = static_cast<ZHM3GameData*>(Glacier::g_pGameData);
        if (pGameData != nullptr && pGameData->m_pWeaponUgradeControl != nullptr)
        {
            const EWeaponType eWeaponType = ZHM3WeaponUpgradeControl::GetWeaponType(pTemplate->Name());
            pGameData->m_pWeaponUgradeControl->ApplyUpgrades(eWeaponType, this, pTemplate->m_pUpgrades, a1 != 0);
        }

        UpdateWeaponPartDrawStatus();
    }

    // PC 0x650520.
    void ZHM3ItemWeaponCustom::ClearUpgrades()
    {
        ZHM3ItemWeaponCustomTemplate* pTemplate = GetHM3WeaponTemplate();
        ZASSERT(pTemplate != nullptr);

        m_VisibleBones.Clear();

        ZHM3GameData* pGameData = static_cast<ZHM3GameData*>(Glacier::g_pGameData);
        if (pGameData != nullptr && pGameData->m_pWeaponUgradeControl != nullptr)
        {
            const EWeaponType eWeaponType = ZHM3WeaponUpgradeControl::GetWeaponType(pTemplate->Name());
            pGameData->m_pWeaponUgradeControl->ApplyDefaultUpgrades(eWeaponType, this);
        }

        pTemplate->ClearUpgrades();

        // PC 0x650520 / iOS 0x1000C4A54. The item template virtual at vtable +0x2DC is
        // ZItemTemplateWeapon::SelectNextWeaponOperation (PC slot 183, 0x511F00); its result is fed to
        // the item's SetWeaponOperation override (PC slot 180, 0x64A850), after which the magazine is
        // refilled to the restored capacity:
        //   SelectNextWeaponOperation(WO_MANUAL) -> SetWeaponOperation(...)
        //   SetProjectilesInMagazine(GetProjectilesPerMagazine())
        SetWeaponOperation(pTemplate->SelectNextWeaponOperation(Glacier::EWeaponOperation::WO_MANUAL));
        SetProjectilesInMagazine(GetProjectilesPerMagazine());

        UpdateWeaponPartDrawStatus();
    }

    // PC 0x64C9D0.
    float ZHM3ItemWeaponCustom::GetPrecisionDegrees()
    {
        Glacier::ZItemTemplateWeapon* pTemplate = static_cast<Glacier::ZItemTemplateWeapon*>(GetItemTemplate());
        ZASSERT(pTemplate != nullptr);

        return pTemplate->GetPrecisionDegrees();
    }

    // PC 0x64CA20. The custom weapon's near damage comes from its (custom) ammo template.
    float ZHM3ItemWeaponCustom::GetNearDamage()
    {
        Glacier::ZItemTemplateAmmo* pAmmo = GetAmmoTemplate();
        ZASSERT(pAmmo != nullptr);

        return pAmmo->GetNearDamage();
    }

    // PC 0x64CA70.
    float ZHM3ItemWeaponCustom::GetFarDamage()
    {
        Glacier::ZItemTemplateAmmo* pAmmo = GetAmmoTemplate();
        ZASSERT(pAmmo != nullptr);

        return pAmmo->GetFarDamage();
    }

    // vtbl slot 180  PC 0x64A850. Overrides ZItemWeapon::SetWeaponOperation so the custom template's
    // m_CustomData.m_eWeaponOperation stays in sync with the item's own operation.
    void ZHM3ItemWeaponCustom::SetWeaponOperation(Glacier::WEAPONOPERATION weaponOperation)
    {
        ZHM3ItemWeaponCustomTemplate* pTemplate = GetHM3WeaponTemplate();
        ZASSERT(pTemplate != nullptr);

        pTemplate->m_CustomData.m_eWeaponOperation = weaponOperation;
        m_eWeaponOperation = weaponOperation;
    }

    // PC 0x64C500 (symbol ZHM3ItemWeaponCustom::GetHM3WeaponTemplate).
    ZHM3ItemWeaponCustomTemplate* ZHM3ItemWeaponCustom::GetHM3WeaponTemplate()
    {
        return static_cast<ZHM3ItemWeaponCustomTemplate*>(GetItemTemplate());
    }

    // PC 0x64C500. Skips the first bone definition and compares names case-insensitively.
    int32_t ZHM3ItemWeaponCustom::GetBoneIdFromBoneName(const char* psBoneName) const
    {
        if (m_pGround == nullptr)
            return 0;

        ZASSERT((m_pGround->GetObjectId() & Glacier::ZLNKOBJ::m_Mask) == Glacier::ZLNKOBJ::m_Id);

        const Glacier::SBoneDefinition* pDefinitions = m_pGround->GetBoneDefinitions();
        const uint32_t lActiveBones = m_pGround->NumActiveBones();
        if (lActiveBones <= 1)
            return 0;

        for (uint32_t i = 1; i < lActiveBones; ++i)
        {
            if (Glacier::striwcmp(pDefinitions[i].Name, psBoneName) == 0)
                return pDefinitions[i].Id;
        }

        return 0;
    }

    // PC 0x64F210. The bone is only appended when not already present in the visible list.
    void ZHM3ItemWeaponCustom::AddVisibleBone(const char* psBoneName)
    {
        const uint32_t lBoneId = static_cast<uint32_t>(GetBoneIdFromBoneName(psBoneName));
        if (m_VisibleBones.Find(&lBoneId) == -1)
            m_VisibleBones.Push(lBoneId);
    }
}
