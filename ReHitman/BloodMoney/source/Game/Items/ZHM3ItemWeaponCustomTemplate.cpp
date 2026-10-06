#include <BloodMoney/Game/Items/ZHM3ItemWeaponCustomTemplate.h>

#include <BloodMoney/Game/Items/ZHM3ItemWeaponCustom.h>
#include <BloodMoney/Game/Items/ZHM3ItemTemplateAmmoCustom.h>
#include <BloodMoney/Game/ZHM3GameData.h>
#include <BloodMoney/Game/ZHM3WeaponUpgradeControl.h>

#include <Glacier/Data/ZGameData.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/ZUniAssert.h>


namespace Hitman
{
    ZHM3ItemWeaponCustomTemplate::ZHM3ItemWeaponCustomTemplate(const char* psName, Glacier::ZBaseGeom* pBaseGeom)
        : ZHM3ItemTemplateWeapon(psName, pBaseGeom)
        , m_pUpgrades(nullptr)
        , m_pLargeClipParticleControl(nullptr)
        , m_pShellParticleControl(nullptr)
    {
    }

    // vtbl slot 12 (RTTI)  PC 0x64EA70
    const Glacier::RTP::ZPropertyInfo& ZHM3ItemWeaponCustomTemplate::GetProperties() const
    {
        return ZHM3ItemWeaponCustomTemplate::Info;
    }

    // vtbl slot 13 (RTTI)  PC 0x64EAE0
    uint32_t ZHM3ItemWeaponCustomTemplate::GetObjectId() const
    {
        return ZHM3ItemWeaponCustomTemplate::m_Id;
    }

    // vtbl slot 14 (RTTI)  PC 0x64EAF0
    void ZHM3ItemWeaponCustomTemplate::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZHM3ItemWeaponCustomTemplate::m_Id;
        mask = ZHM3ItemWeaponCustomTemplate::m_Mask;
    }

    // vtbl slot 15 (RTTI)  PC 0x64EA80 -> &ZHM3ItemWeaponCustomTemplate::m_OldClassInfo
    Glacier::ZGEOMCLASSINFO* ZHM3ItemWeaponCustomTemplate::GetOldClassInfo() const
    {
        return ZHM3ItemWeaponCustomTemplate::m_OldClassInfo;
    }

    // PC 0x64EAA0.
    void* ZHM3ItemWeaponCustomTemplate::GetCustomData()
    {
        return &m_CustomData;
    }

    // PC 0x64A6C0. Stores the value into the base ZItemTemplateWeapon muzzle velocity.
    void ZHM3ItemWeaponCustomTemplate::SetMuzzleVelocity(float fVelocity)
    {
        m_fMuzzleVelocity = fVelocity;
    }

    // PC 0x64A720.
    float ZHM3ItemWeaponCustomTemplate::GetImpact() const
    {
        return m_fImpact;
    }

    // PC 0x64C2C0. Clears the applied upgrade table and restores the stored default values.
    void ZHM3ItemWeaponCustomTemplate::ClearUpgrades()
    {
        ZASSERT(m_pUpgrades != nullptr);
        m_pUpgrades->Clear();

        ApplyDefaultValues();

        // PC 0x64C2C0 tail: when the weapon's first ammo template is a ZHM3ItemTemplateAmmoCustom,
        // re-apply the custom ammo template's stored defaults to its live ammo fields.
        Glacier::ZItemTemplateAmmo* pAmmo = GetAmmoTemplate(0);
        if (pAmmo != nullptr &&
            (ZHM3ItemTemplateAmmoCustom::m_Mask & pAmmo->GetObjectId()) == ZHM3ItemTemplateAmmoCustom::m_Id)
        {
            ZHM3ItemTemplateAmmoCustom* pCustomAmmo = static_cast<ZHM3ItemTemplateAmmoCustom*>(pAmmo);
            pCustomAmmo->m_lProjectilesPerMagazine = pCustomAmmo->m_DefaultValues.m_lProjectilesPerMagazine;
            pCustomAmmo->m_lProjectilesPerShot     = pCustomAmmo->m_DefaultValues.m_lProjectilesPerShot;
            pCustomAmmo->m_fNearDamage             = pCustomAmmo->m_DefaultValues.m_fNearDamage;
            pCustomAmmo->m_fFarDamage              = pCustomAmmo->m_DefaultValues.m_fFarDamage;
            pCustomAmmo->m_bCanPenetrate           = pCustomAmmo->m_DefaultValues.m_bCanPenetrate;
        }
    }

    // PC 0x64A4B0. Restores the live weapon/template fields from the stored defaults and resets
    // the custom-weapon flags.
    void ZHM3ItemWeaponCustomTemplate::ApplyDefaultValues()
    {
        // The PC body first resets the runtime custom data (+0x1AC..+0x1BD): silencer/scope/ammo are
        // cleared, the weapon operation is forced to semi-auto, and the flag byte keeps only the
        // flashlight bit and forces the magazine bit.
        m_CustomData.m_eSilencerType = static_cast<ESilencerType>(0);
        m_CustomData.m_eScopeType = static_cast<EScopeType>(0);
        m_CustomData.m_eAmmoType = static_cast<EAmmoType>(0);
        m_CustomData.m_eWeaponOperation = Glacier::EWeaponOperation::WO_SEMIAUTO;

        m_CustomData.m_bNightVision = false;
        m_CustomData.m_bLaserIndicator = false;
        m_CustomData.m_bRedDot = false;
        // m_bFlashLight is preserved.
        m_CustomData.m_bDoubleCapacityAmmo = false;
        m_CustomData.m_bHasMagazine = true;
        m_CustomData.m_bBiPod = false;
        m_CustomData.m_bLightWeightFrame = false;
        m_CustomData.m_bBarrelExtension = false;
        m_CustomData.m_bReloadBoost = false;
        m_CustomData.m_bTrippleAmmo = false;

        // Then every stored default is copied back into the matching live template field.
        m_WeaponOperations.SetBitfield(m_DefaultValues.m_WeaponOperations);
        m_fMuzzleVelocity = m_DefaultValues.m_fMuzzleVelocity;
        m_fPrecisionDeg = m_DefaultValues.m_fPrecisionDeg;
        m_fNearRange = m_DefaultValues.m_fNearRange;
        m_fFarRange = m_DefaultValues.m_fFarRange;
        m_fDamageMultiplier = m_DefaultValues.m_fDamageMultiplier;
        m_fImpact = m_DefaultValues.m_fImpact;
        m_fTimeBetweenShots = m_DefaultValues.m_fTimeBetweenShot;
        m_bCanFireProjectiles = m_DefaultValues.m_bCanFireProjectiles;
        m_bCanHaveMagazines = m_DefaultValues.m_bCanHaveMagazines;
        m_bSniperMode = m_DefaultValues.m_bSniperMode;
        m_eHM3RecoilRandom = static_cast<EHM3RecoilRandom>(m_DefaultValues.m_eHM3RecoilRandom);
        m_fRecoilStrengthX = m_DefaultValues.m_fRecoilStrengthX;
        m_fRecoilStrengthY = m_DefaultValues.m_fRecoilStrengthY;
        m_eScope = static_cast<EHM3WeaponScope>(m_DefaultValues.m_eScope);
        m_eSilencerType = static_cast<ESilencerType>(m_DefaultValues.m_eSilencerType);

        // PC tail: the custom sniper always uses the large scope.
        if (GetHM3ItemType() == EHM3ItemType::eHM3CustomSniper)
            m_CustomData.m_eScopeType = eLargeScope;
    }

    // PC 0x653AF0. The PC symbol lives on ZHM3WeaponUpgradeControl; the PS2 spin (0x5E4134) exposes
    // this thin wrapper on the template, which is what the item routes through.
    void ZHM3ItemWeaponCustomTemplate::ApplyUpgrades(ZHM3ItemWeaponCustom* pCustomGun)
    {
        ZHM3GameData* pGameData = static_cast<ZHM3GameData*>(Glacier::g_pGameData);
        if (pGameData == nullptr || pGameData->m_pWeaponUgradeControl == nullptr)
            return;

        const EWeaponType eWeaponType = ZHM3WeaponUpgradeControl::GetWeaponType(Name());
        pGameData->m_pWeaponUgradeControl->ApplyUpgrades(eWeaponType, pCustomGun, m_pUpgrades, false);
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        // PC 0x80FA30 (game offset 0x19C). The PS2 property record names this entry "m_pUpgrades";
        // it is the only property of the chain (Info.First == 0x80FA30).
        static Glacier::RTP::ZDataProperty<Glacier::REFTAB*> Upgrades{
            .m_Node = {.m_Next = nullptr, .m_Name = "m_pUpgrades", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_REFTAB_ptr,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemWeaponCustomTemplate, m_pUpgrades)};
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZHM3ItemWeaponCustomTemplate, // ClassName
        ZHM3ItemTemplateWeapon,       // BaseClass
        0x009B16F8,                   // OldClassInfoAddr
        "ZHM3ItemWeaponCustomTemplate", // FactoryName
        0x0,                          // FactoryNameAddr (documentation only; not bound by the macro)
        cProperties::Upgrades,        // FirstProperty
        0x0080FA44,                   // PropertiesAddr (ZHM3ItemWeaponCustomTemplate::Info)
        0x009B1510,                   // IdAddr (ZHM3ItemWeaponCustomTemplate::m_Id)
        0x009B1514                    // MaskAddr (ZHM3ItemWeaponCustomTemplate::m_Mask)
    );
#   pragma endregion
}
