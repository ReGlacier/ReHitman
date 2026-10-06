#pragma once

#include <Glacier/Items/ZItem.h>
#include <Glacier/Items/EWeaponOperation.h>
#include <Glacier/GlacierFWD.h>

namespace Glacier
{
    // Engine globals defined in ZItemWeapon.cpp (source engine/geomsextend/zitem.cpp). The PC
    // build keeps both as plain engine flags rather than debug objects:
    //   g_bIsInfClip  PC 0x99BF28 -- the ZCheatMenu "InfClip" toggle, read by GetProjectilesInMagazine.
    //   g_lBlockFire  PC 0x99BF2C -- debug gate that early-outs ZItemWeapon::FireRound.
    extern bool g_bIsInfClip;
    extern int g_lBlockFire;

    class ZItemWeapon : public ZItem
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZItemWeapon, 0x1007D2u);

        // methods
        ZItemWeapon(const char* psName, ZBaseGeom* pBaseGeom);

        // vtbl (ZGEOM/ZItem overrides)
        ~ZItemWeapon() override;
        const RTP::ZPropertyInfo& GetProperties() const override;
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void CopyData(const ZGEOM* Source) override;

        //vftable
        virtual void DestroyItem();
        virtual void SetAmmoTemplate(ZItemTemplateAmmo*);
        virtual ZItemTemplateAmmo* GetAmmoTemplate();
        virtual void SetTarget(const Glacier::ZVector3* target);
        virtual bool GetTarget(Glacier::ZVector3* targetPos); // return true is result vector isn't zeroed
        virtual void* Target();
        virtual WEAPONOPERATION GetWeaponOperation();
        virtual void SetWeaponOperation(WEAPONOPERATION weaponOperation);
        virtual void SelectNextWeaponOperation();
        virtual void GetFirePosition(ZMat3x3* mat, ZVector3* pos);
        virtual ZGEOM* GetMuzzleExitPos();
        virtual void GetRootMuzzleExitPos(const Glacier::ZVector3* result);
        virtual ZItemTemplateWeapon* GetWeaponTemplate();
        virtual bool GetBulletInChamber();
        virtual void SetBulletInChamber(bool value);
        virtual int GetProjectilesInMagazine(); //Could be overridden by cheat 'Inf Ammo'
        virtual void SetProjectilesInMagazine(int value);
        virtual int GetProjectilesPerMagazine();
        virtual bool WeaponReady();
        virtual void SetUseBulletsFromMagazine(bool);
        virtual int GetNumOfBulletsPerReloadCycle();
        virtual void ReloadStart();
        virtual void ReloadEnd();
        virtual void ReloadStartActivateAnimation();
        virtual void ChamberStart();
        virtual void ChamberEnd();
        virtual void FireStart();
        virtual void FireEnd();
        virtual void FireRound();
        virtual void EmptyFire();
        virtual void FireRoundActivateAnimation();
        virtual void OnActivateProjectile(ZGEOM*);
        virtual void CopyGeom(ZGEOM* from, ZGEOM* unused, ZGROUP* inGroup, bool makeActive);

        //data (total size is 0xDC, ZItem size is 0x84)
        Glacier::ZVector3 m_vTarget;
        Glacier::ZREF m_rAmmoTemplate;
        int m_lProjectilesInMagazine;
        bool m_bBulletInChamber;
        RE_ADD_PADDING(3);
        // Verified from the PC ctor: `mov dword ptr [esi+0x9C], 0` and the RTP vtbl is Data_int.
        int m_lBurstCount;
        // Verified from the PC ctor: the four byte writes land at [esi+0xA0..0xA3].
        bool m_bRequestFireRelease;
        bool m_bReloading;
        bool m_bChambering;
        bool m_bTriggerHeld;
        // Verified from PC ReloadEnd: `fstp dword ptr [esi+0xA4]`.
        float m_fTimeLastShot;
        EWeaponOperation m_eWeaponOperation;
        REFTAB* m_prtWeaponParts;
        int m_rParticleController;
        int m_lMuzzleFireIndex;
        int m_lMuzzleSmokeIndex;
        int m_lCartridgeIndex;
        Glacier::ZREF m_rMuzzleLight;
        Glacier::ZVector3 m_vMuzzleLightAlign;
        Glacier::ZREF m_rSlide;
        Glacier::ZREF m_rClip;
        bool m_useBulletsFromMagazine;
        bool m_bWantSoundEvent;
        RE_ADD_PADDING(2);
    };
    RE_VERIFY_SIZE(ZItemWeapon, 0xDC); // Verified

}
