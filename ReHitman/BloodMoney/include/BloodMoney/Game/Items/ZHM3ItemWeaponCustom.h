#pragma once

#include <BloodMoney/Game/Items/ZHM3ItemWeapon.h>
#include <BloodMoney/Game/Items/ESilencerType.h>
#include <Glacier/ZSTL/ZStackArray.h>

namespace Hitman
{
    class ZHM3ItemWeaponCustomTemplate;

    using ZStackArrayVisibleBones = Glacier::ZStackArray<20, uint32_t>;

    class ZHM3ItemWeaponCustom : public ZHM3ItemWeapon
    {
    public:
        // vftable
        virtual void GetAnims(); //Load anims inside
        virtual void* GetMuzzleVelocity(float);

        // vftable override (base ZItemWeapon slot 180, PC 0x64A850): keeps the custom template's
        // m_CustomData.m_eWeaponOperation in sync with the item's own operation.
        void SetWeaponOperation(Glacier::WEAPONOPERATION weaponOperation) override;

        // api
        void ApplyUpgrades(char a1);
        void UpdateWeaponPartDrawStatus();
        void ClearUpgrades();
        void SetSilencerType(ESilencerType type);

        // PC 0x64C500 / 0x64F210. Bones are addressed through the ground link object
        // (ZHM3ItemWeapon::m_pGround); GetBoneIdFromBoneName looks a bone up in its definitions.
        int32_t GetBoneIdFromBoneName(const char* psBoneName) const;
        void AddVisibleBone(const char* psBoneName);

        // helpers (PC). Kept non-virtual so the verified vtable/layout is untouched.

        // PC 0x64F190. Hides (or shows) the bone on the ground link object; the custom hardballer
        // pickup additionally toggles it on the Ground01/Ground02 sub-geoms.
        void HideBone(uint32_t lBoneId, bool bHide);
        // PC 0x64C480 / 0x64F260.
        void RemoveVisibleBone(uint32_t lBoneId);
        void RemoveVisibleBone(const char* psBoneName);
        // PC 0x64C9D0 / 0x64CA20 / 0x64CA70, forwarded to the weapon/ammo template.
        float GetPrecisionDegrees();
        float GetNearDamage();
        float GetFarDamage();
        // PC symbol `ZHM3ItemWeaponCustom::GetHM3WeaponTemplate`; this item's own template.
        ZHM3ItemWeaponCustomTemplate* GetHM3WeaponTemplate();

        // data (new size is 0x1C0)
        ZStackArrayVisibleBones m_VisibleBones;
        Glacier::Animation::Header* m_pAnimReloadBoltAction;
        Glacier::Animation::Header* m_pAnimReloadDoubleCapMag;
        Glacier::Animation::Header* m_pAnimReloadBeltFeeding;
        int32_t m_nNumOfReloads;
    };
    RE_VERIFY_SIZE(ZHM3ItemWeaponCustom, 0x1C0); // Verified
}