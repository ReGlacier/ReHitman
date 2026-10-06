#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/Items/ZItemTemplate.h>
#include <Glacier/Items/EWeaponOperation.h>
#include <Glacier/ZSTL/ZBitfield.h>
#include <Glacier/Audio/ZSDOwner.h>

namespace Glacier
{
    enum WEAPONTYPE : uint32_t {
        WT_PISTOL = 0,
        WT_REVOLVER = 1,
        WT_SUBMACHINEGUN = 2,
        WT_MACHINEGUN = 3,
        WT_RIFLE = 4,
        WT_PUMPGUN = 5,
        WT_SHOTGUN = 6,
        WT_ROCKETLAUNCHER = 7,
        WT_KNIFE = 8,
        WT_PIANO = 9,
        WT_OTHER = 10,
        WT_GRENADE = 11,
        WT_MOLOTOV = 12,
        WT_CLOSECOMBAT = 13,
        WT_FORCE32 = 2147483647u,
    };


    class ZItemTemplateWeapon : public ZItemTemplate
    {
    public:
        // RTTI
        DECLARE_GEOM_CLASS(ZItemTemplateWeapon, 0x1007D4u);

        // methods
        ZItemTemplateWeapon(const char* psName, ZBaseGeom* pBaseGeom);

        // vtbl (ZGEOM/ZItemTemplate overrides)
        ~ZItemTemplateWeapon() override;
        const RTP::ZPropertyInfo& GetProperties() const override;
        uint32_t GetObjectId() const override;
        void GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const override;
        ZGEOMCLASSINFO* GetOldClassInfo() const override;
        void ClassInit() override;
        void CopyData(const ZGEOM* Source) override;
        ZItem* CreateItem(ZGROUP* pGroup, unsigned int iGeomResourceId, bool bOverrideVisibleForNPC, bool bVisibleForNPC) override;
        uint32_t GetItemClassId() const override;
        void ModifyState(CCom* pCom) override;

        // vftable (ZItemTemplateWeapon-own virtuals, slots [166, 192))
        virtual void CreateItemAndActuallyUseDestinationParameter(ZGROUP*, unsigned int);
        virtual void DestroyItem(ZItem*); //actually not destroying anything :sadpeppe:
        virtual float GetRecoil();
        virtual bool CanHaveMagazines(); //returns same to CanFireProjectiles
        virtual bool CanFireProjectiles();
        virtual void SetCanFireProjectiles(bool);
        virtual bool HasSniperMode();
        virtual int GetWeaponType();
        virtual EWeaponOperation GetWeaponOperations();
        virtual float GetTimeBetweenShots();
        virtual ZItemTemplateAmmo* GetAmmoTemplate(int ammoIndex);
        virtual float GetMuzzleVelocity();
        virtual float GetNearRange();
        virtual float GetFarRange();
        virtual float GetPrecisionDegrees();
        virtual int GetDefaultProjectilesPerMagazine();
        virtual float GetCartridgeSpeed();
        virtual WEAPONOPERATION SelectNextWeaponOperation(WEAPONOPERATION weaponOperation);
        virtual double CalcDamage(const ZItemWeapon* item, float distance);
        virtual double CalcImpact(const ZItemWeapon* item, float distance);
        virtual REFTAB* GetWeaponParts();
        virtual ZGEOM* GetMuzzleFire();
        virtual ZGEOM* GetMuzzleSmoke();
        virtual ZGEOM* GetMuzzleLight();
        virtual ZGEOM* GetCartridge(); //Same to GetMuzzleSmoke
        virtual ZGEOM* GetMuzzleEffect();

        // data (total size is 0x15C, ZItemTemplate size is 0x74)
        ZSDOwner m_SoundDef;
        ZMSGID m_msgSetSniperQuality;
        ZMSGID m_msgSetSniperOrgGeometryMat;
        ZMSGID m_msgSetSniperOrgGeometryPos;
        ZMSGID m_msgSetSniperOverlay;
        ZREF m_rMuzzleSmoke;
        ZREF m_rMuzzleFire;
        ZREF m_rMuzzleLight;
        ZREF m_rMuzzleSmokeAlign;
        ZREF m_rMuzzleFireAlign;
        ZBitfield<EWeaponOperation> m_WeaponOperations;
        WEAPONTYPE m_eWeaponType;
        REFTAB m_AmmoTemplate;
        float m_fMuzzleVelocity;
        float m_fPrecisionDeg;
        float m_fNearRange;
        float m_fFarRange;
        float m_fDamageMultiplier;
        float m_fImpact;
        float m_fTimeBetweenShots;
        bool m_bCanFireProjectiles;
        char m_bCanHaveMagazines;
        bool m_bSniperMode;
        RE_ADD_PADDING(1);
        REFTAB m_WeaponParts;
        ZREF m_rWeaponFlash;
        uint32_t m_lNumInstances;
        float m_fCartridgeSpeed;
        ZMat3x3 m_mFireRelative;
        ZVector3 m_vFireRelative;
        float m_fFireLength;
        float m_fFireAngle;
        ZItemWeapon** m_pWeaponInstances;
        uint32_t m_lInstanceIndex;
        ZGEOM* m_pProjectileAlign;
        ZGEOM* m_pCartridgeAlign;
        ZGEOM* m_pMuzzleFlashAlign;
        ZGEOM* m_pMuzzleSmokeAlign;
    };
    RE_VERIFY_SIZE(ZItemTemplateWeapon, 0x15C); // Verified
}
