#pragma once

#include <BloodMoney/Game/Items/ZHM3ItemWeapon.h>
#include <BloodMoney/Game/Items/ESilencerType.h>
#include <Glacier/ZSTL/ZStackArray.h>

namespace Hitman::BloodMoney
{
    using ZStackArrayVisibleBones = Glacier::ZStackArray<20, uint32_t>;

    class ZHM3ItemWeaponCustom : public ZHM3ItemWeapon
    {
    public:
        // vftable
        virtual void GetAnims(); //Load anims inside
        virtual void* GetMuzzleVelocity(float);

        // api
        void ApplyUpgrades(char a1);
        void UpdateWeaponPartDrawStatus();
        void ClearUpgrades();
        void SetSilencerType(ESilencerType type);

        // data (new size is 0x1C0)
        ZStackArrayVisibleBones m_VisibleBones;
        Glacier::Animation::Header* m_pAnimReloadBoltAction;
        Glacier::Animation::Header* m_pAnimReloadDoubleCapMag;
        Glacier::Animation::Header* m_pAnimReloadBeltFeeding;
        int32_t m_nNumOfReloads;
    };
    RE_VERIFY_SIZE(ZHM3ItemWeaponCustom, 0x1C0); // Verified
}