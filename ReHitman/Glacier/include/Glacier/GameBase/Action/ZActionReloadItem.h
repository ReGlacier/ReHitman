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

    class ZActionReloadItem : public ZLnkAction
    {
    public:
        ~ZActionReloadItem() override;
        bool Execute() override;
        void Terminate() override;
        bool Update() override;
        const char* Name() const override;
        virtual Animation::Header* GetReloadAnim(ZItemTemplateWeapon* pWeapon);
        virtual uint32_t GetReloadAnimId(Animation::Header* pAnim);
        virtual Animation::Header* GetReloadAnimFromId(uint32_t animationId);
        virtual float GetStartTime() const;
        virtual ZIKHAND* GetHand() const;
        virtual ZLNKWHANDS* GetActor() const;
        bool SupportsLoadSave() override;
        void LoadSave(ISerializerStream& stream, bool bSaving) override;
        bool WeaponReloaded(ZItemWeapon* pWeapon) const;
        void ReloadWeapon(ZItemWeapon* pWeapon, int lAmmo);
        void SetSingleLoadStartEnd();

        ZActionReloadItem(uint32_t actionId, ZLNKWHANDS* pActor, ZIKHAND* pHand,
                          Animation::Header* pAnimReload, bool bMirrorAnim);
        ZActionReloadItem(uint32_t actionId, ZLNKWHANDS* pActor, ZIKHAND* pHand,
                          Animation::Header* pAnimReloadRevolver, Animation::Header* pAnimReloadPistol,
                          Animation::Header* pAnimReloadSubMachineGun, Animation::Header* pAnimReloadRifle,
                          Animation::Header* pAnimReloadShotgun, Animation::Header* pAnimReloadPumpgun,
                          Animation::Header* pAnimReloadRPG);

        ZLNKWHANDS* m_pActor;
        ZIKHAND* m_pHand;
        Animation::Header* m_pAnimReloadRevolver;
        Animation::Header* m_pAnimReloadPistol;
        Animation::Header* m_pAnimReloadSubMachineGun;
        Animation::Header* m_pAnimReloadRifle;
        Animation::Header* m_pAnimReloadPumpgun;
        Animation::Header* m_pAnimReloadShotgun;
        Animation::Header* m_pAnimReloadRPG;
        Animation::Header* m_pCurrentReloadAnim;
        float m_fReloadStartTime;
        float m_fCycleTime;
        float m_fSingleLoadTimeStart;
        float m_fSingleLoadTimeEnd;
        bool m_bMirrorAnim;
        bool m_bFinishInNextCycle;
        bool m_bFinishCycle;
        RE_ADD_PADDING(1);
    };
    RE_VERIFY_SIZE(ZActionReloadItem, 0x44);
}
