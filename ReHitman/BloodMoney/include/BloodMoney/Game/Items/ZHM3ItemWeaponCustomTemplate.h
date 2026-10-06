#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/Animation/ZAnimVariationHandle.h>
#include <Glacier/Items/EWeaponOperation.h>
#include <BloodMoney/Game/Items/ZHM3ItemTemplateWeapon.h>
#include <BloodMoney/Game/Items/ESilencerType.h>
#include <BloodMoney/Game/Items/EScopeType.h>
#include <BloodMoney/Game/Items/EAmmoType.h>


#include <BloodMoney/Game/ZHM3ClipParticleControl.h>

namespace Hitman
{
    class ZHM3ItemWeaponCustom;

    class ZHM3ItemWeaponCustomTemplate : public ZHM3ItemTemplateWeapon
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZHM3ItemWeaponCustomTemplate, 0x10042Cu);

        // methods
        ZHM3ItemWeaponCustomTemplate(const char* psName, Glacier::ZBaseGeom* pBaseGeom);

        // vtbl (RTTI)
        const Glacier::RTP::ZPropertyInfo& GetProperties() const override;
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        Glacier::ZGEOMCLASSINFO* GetOldClassInfo() const override;

        // types
        struct SCustomWeaponData {
            ESilencerType m_eSilencerType;
            EScopeType m_eScopeType;
            Glacier::EWeaponOperation m_eWeaponOperation;
            EAmmoType m_eAmmoType;
            bool m_bNightVision : 1;
            bool m_bLaserIndicator : 1;
            bool m_bRedDot : 1;
            bool m_bFlashLight : 1;
            bool m_bDoubleCapacityAmmo : 1;
            bool m_bHasMagazine : 1;
            bool m_bBiPod : 1;
            bool m_bLightWeightFrame : 1;
            bool m_bBarrelExtension : 1;
            bool m_bReloadBoost : 1;
            bool m_bTrippleAmmo : 1;
        };
        RE_VERIFY_SIZE(SCustomWeaponData, 0x14);

        struct SDefaultValues
        {
            uint32_t m_WeaponOperations;
            float    m_fMuzzleVelocity;
            float    m_fPrecisionDeg;
            float    m_fNearRange;
            float    m_fFarRange;
            float    m_fDamageMultiplier;
            float    m_fImpact;
            float    m_fTimeBetweenShot;
            bool     m_bCanFireProjectiles;
            bool     m_bCanHaveMagazines;
            bool     m_bSniperMode;
            RE_ADD_PADDING(1); 
            int32_t  m_eHM3RecoilRandom;
            float    m_fRecoilStrengthX;
            float    m_fRecoilStrengthY;
            int32_t  m_eScope;
            int32_t  m_eSilencerType;   
        };
        RE_VERIFY_SIZE(SDefaultValues, 0x38);

        /// vftable
        virtual void* GetCustomData();
        virtual void SetMuzzleVelocity(float);

        /// api (PC). Kept non-virtual so the verified vtable/layout is untouched.

        // PC 0x653AF0 (declared through the PS2 spin as ZHM3ItemWeaponCustomTemplate::ApplyUpgrades,
        // PS2 0x5E4134): walks m_pUpgrades and applies every present upgrade to pCustomGun.
        void ApplyUpgrades(ZHM3ItemWeaponCustom* pCustomGun);
        // PC 0x64A4B0. Restores the live weapon/template fields from m_DefaultValues.
        void ApplyDefaultValues();
        // PC 0x64C2C0. Clears m_pUpgrades and restores the stored default values.
        void ClearUpgrades();
        // PC 0x64A720. Returns the (base template) impact.
        float GetImpact() const;

        /// data (total size is 0x1FC, base size is 0x1A0)
        Glacier::REFTAB* m_pUpgrades;
        ZHM3ClipParticleControl* m_pLargeClipParticleControl;
        ZHM3ClipParticleControl* m_pShellParticleControl;
        SCustomWeaponData m_CustomData;
        SDefaultValues m_DefaultValues;
        Glacier::ZAnimVariationHandle m_CustomHBAnimHandle;
    };
    RE_VERIFY_SIZE(ZHM3ItemWeaponCustomTemplate, 0x1FC); // Verfied
}