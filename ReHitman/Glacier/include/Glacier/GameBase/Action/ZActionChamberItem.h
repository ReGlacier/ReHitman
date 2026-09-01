#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/IK/ZLnkAction.h>

namespace Glacier
{
    class ZItemWeapon;
    class ZItemTemplateWeapon;
    class ZLNKWHANDS;
    class ZIKHAND;
    namespace Animation { struct Header; }

    class ZActionChamberItem : public ZLnkAction
    {
    public:
        ~ZActionChamberItem() override;
        bool Execute() override;
        bool Update() override;
        const char* Name() const override;
        bool SupportsLoadSave() override;
        void LoadSave(ISerializerStream& stream, bool bSaving) override;

        ZActionChamberItem(uint32_t actionId, ZLNKWHANDS* pActor, ZIKHAND* pHand,
                           Animation::Header* pAnimChamberGun, Animation::Header* pAnimChamberRifle,
                           Animation::Header* pAnimChamberShotgun, Animation::Header* pAnimChamberPumpgun,
                           Animation::Header* pAnimChamberRPG, Animation::Header* pAnimChamberGrenade);
        Animation::Header* GetChamberAnim(ZItemTemplateWeapon* pWeapon);
        void ChamberWeapon(ZItemWeapon* pWeapon);

        ZLNKWHANDS* m_pActor;
        ZIKHAND* m_pHand;
        Animation::Header* m_pAnimChamberGun;
        Animation::Header* m_pAnimChamberRifle;
        Animation::Header* m_pAnimChamberPumpgun;
        Animation::Header* m_pAnimChamberShotgun;
        Animation::Header* m_pAnimChamberRPG;
        Animation::Header* m_pAnimChamberGrenade;
        Animation::Header* m_pCurrentChamberAnim;
        float m_fChamberStartTime;
    };
    RE_VERIFY_SIZE(ZActionChamberItem, 0x30);
}
