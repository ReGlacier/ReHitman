#include <Glacier/Items/ZItemTemplateWeapon.h>

#include <Glacier/Com/CCom.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/Items/ZItem.h>
#include <Glacier/Items/ZItemTemplateAmmo.h>
#include <Glacier/Items/ZItemWeapon.h>
#include <Glacier/RTP/Base.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/Runtime/ZGEOMCLASSINFO.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZSTL/ZMath.h>


namespace Glacier
{
    // vtbl slot 82  PC 0x50F280
    ZItemTemplateWeapon::ZItemTemplateWeapon(const char* psName, ZBaseGeom* pBaseGeom)
        : ZItemTemplate(psName, pBaseGeom)
        , m_AmmoTemplate(8, 0)
        , m_WeaponParts(8, 0)
    {
        m_rMuzzleSmoke = 0;
        m_rMuzzleFire = 0;
        m_rMuzzleLight = 0;
        m_WeaponOperations.Reset();
        m_eWeaponType = WT_PISTOL;
        m_fMuzzleVelocity = 0.0f;
        m_fPrecisionDeg = 0.0f;
        m_fNearRange = 0.0f;
        m_fFarRange = 0.0f;
        m_fDamageMultiplier = 0.0f;
        m_fImpact = 0.0f;
        m_fTimeBetweenShots = 0.5f;
        m_bCanFireProjectiles = false;
        m_bCanHaveMagazines = 1;
        m_bSniperMode = false;
        m_rWeaponFlash = 0;
        m_lInstanceIndex = 0;

        m_vFireRelative = ZVector3(0.0f, 0.0f, 0.0f);
        m_fFireLength = 0.0f;
        m_fFireAngle = 0.0f;
        mreset(m_mFireRelative.data);

        m_msgSetSniperQuality = g_pEngineData->RegisterZMsg("SniperMainCamera_SetQuality", 0, __FILE__, __LINE__);
        m_msgSetSniperOrgGeometryMat = g_pEngineData->RegisterZMsg("SniperMainCamera_SetOrgMatrix", 0, __FILE__, __LINE__);
        m_msgSetSniperOrgGeometryPos = g_pEngineData->RegisterZMsg("SniperMainCamera_SetOrgPosition", 0, __FILE__, __LINE__);
        m_msgSetSniperOverlay = g_pEngineData->RegisterZMsg("SniperMainCamera_SetOverlay", 0, __FILE__, __LINE__);

        m_pProjectileAlign = nullptr;
        m_pCartridgeAlign = nullptr;
        m_pMuzzleFlashAlign = nullptr;
        m_pMuzzleSmokeAlign = nullptr;

        // NOTE: the PC ctor leaves m_rMuzzleSmokeAlign, m_rMuzzleFireAlign, m_lNumInstances,
        //       m_fCartridgeSpeed and m_pWeaponInstances untouched; they are filled by RTP load.
    }

    ZItemTemplateWeapon::~ZItemTemplateWeapon() = default;

    // vtbl slot 12  PC 0x50F220
    const RTP::ZPropertyInfo& ZItemTemplateWeapon::GetProperties() const
    {
        return ZItemTemplateWeapon::Info;
    }

    // vtbl slot 13  PC 0x510A90
    uint32_t ZItemTemplateWeapon::GetObjectId() const
    {
        return ZItemTemplateWeapon::m_Id;
    }

    // vtbl slot 14  PC 0x510AA0
    void ZItemTemplateWeapon::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZItemTemplateWeapon::m_Id;
        mask = ZItemTemplateWeapon::m_Mask;
    }

    // vtbl slot 15  PC 0x50F230
    ZGEOMCLASSINFO* ZItemTemplateWeapon::GetOldClassInfo() const
    {
        return ZItemTemplateWeapon::m_OldClassInfo;
    }

    // vtbl slot 82  PC 0x50F280
    void ZItemTemplateWeapon::ClassInit()
    {
        m_fTimeBetweenShots = 60.0f / m_fTimeBetweenShots;
        m_fMuzzleVelocity *= 100.0f;
        m_fNearRange *= 100.0f;
        m_fFarRange *= 100.0f;

        if (ZGEOM* pMuzzleExit = FindGeom("PosBox_MuzzleExit", nullptr))
            m_pProjectileAlign = pMuzzleExit;
        else
            m_pProjectileAlign = this;

        if (ZGEOM* pCartridgeEject = FindGeom("PosBox_CartridgeEject", nullptr))
            m_pCartridgeAlign = pCartridgeEject;
        if (ZGEOM* pMuzzleSmoke = FindGeom("PosBox_MuzzleSmoke", nullptr))
            m_pMuzzleSmokeAlign = pMuzzleSmoke;
        if (ZGEOM* pMuzzleFlash = FindGeom("PosBox_MuzzleFlash", nullptr))
            m_pMuzzleFlashAlign = pMuzzleFlash;
    }

    // vtbl slot 104  PC 0x511DD0
    void ZItemTemplateWeapon::CopyData(const ZGEOM* Source)
    {
        ZGROUP::CopyData(Source);

        if ((Source->GetObjectId() & ZItemTemplateWeapon::m_Mask) != ZItemTemplateWeapon::m_Id)
            return;

        const ZItemTemplateWeapon* pSource = static_cast<const ZItemTemplateWeapon*>(Source);

        m_WeaponOperations = pSource->m_WeaponOperations;
        m_eWeaponType = pSource->m_eWeaponType;
        m_fMuzzleVelocity = pSource->m_fMuzzleVelocity;
        m_fPrecisionDeg = pSource->m_fPrecisionDeg;
        m_fNearRange = pSource->m_fNearRange;
        m_fFarRange = pSource->m_fFarRange;
        m_fDamageMultiplier = pSource->m_fDamageMultiplier;
        m_fImpact = pSource->m_fImpact;
        m_fTimeBetweenShots = pSource->m_fTimeBetweenShots;
        m_bCanFireProjectiles = pSource->m_bCanFireProjectiles;
        m_bCanHaveMagazines = pSource->m_bCanHaveMagazines;
        m_bSniperMode = pSource->m_bSniperMode;

        RefRun Run;
        pSource->m_AmmoTemplate.RunInitNxtRef(&Run);
        for (const uint32_t* pRef = pSource->m_AmmoTemplate.RunNxtRefPtr(&Run);
             pRef != nullptr;
             pRef = pSource->m_AmmoTemplate.RunNxtRefPtr(&Run))
        {
            m_AmmoTemplate.Add(*pRef);
        }

        m_pProjectileAlign = nullptr;
        m_pCartridgeAlign = nullptr;
    }

    // vtbl slot 147  PC 0x514100. Ignores the destination group and creates into the scene root.
    ZItem* ZItemTemplateWeapon::CreateItem(ZGROUP* /*pGroup*/, unsigned int iGeomResourceId, bool bOverrideVisibleForNPC, bool bVisibleForNPC)
    {
        return ZItemTemplate::CreateItem(g_pEngineData->m_pRoot, iGeomResourceId, bOverrideVisibleForNPC, bVisibleForNPC);
    }

    // vtbl slot 149  PC 0x50F240
    uint32_t ZItemTemplateWeapon::GetItemClassId() const
    {
        return 0x1007D2u;
    }

    // vtbl slot 152  PC 0x50F3D0
    void ZItemTemplateWeapon::ModifyState(CCom* pCom)
    {
        int lState = 0;
        int rItem = 0;
        int lAmmoLeft = 0;

        pCom->GetVal("lState", &lState);
        pCom->GetVal("rItem", &rItem);
        ZGEOM::RefToPtr(static_cast<uint32_t>(rItem));
        pCom->GetVal("lAmmoLeft", &lAmmoLeft);
    }

    // vtbl slot 166  PC 0x514120
    void ZItemTemplateWeapon::CreateItemAndActuallyUseDestinationParameter(ZGROUP* pGroup, unsigned int iGeomResourceId)
    {
        ZItemTemplate::CreateItem(pGroup, iGeomResourceId, false, false);
    }

    // vtbl slot 167  PC 0x50F340
    void ZItemTemplateWeapon::DestroyItem(ZItem* pItem)
    {
        pItem->Delete();
    }

    // vtbl slot 168  PC 0x50F4D0
    float ZItemTemplateWeapon::GetRecoil()
    {
        float fPrecision = m_fPrecisionDeg - 1.0f;

        if (fPrecision >= 0.0f)
        {
            if (fPrecision > 10.0f)
                fPrecision = 10.0f;
        }
        else
        {
            fPrecision = 0.0f;
        }

        const float fRecoil = fPrecision * 0.1f;
        return fRecoil < 0.5f ? 0.5f : fRecoil;
    }

    // vtbl slot 169  PC 0x50F460
    bool ZItemTemplateWeapon::CanHaveMagazines()
    {
        return m_bCanHaveMagazines;
    }

    // vtbl slot 170  PC 0x50F450
    bool ZItemTemplateWeapon::CanFireProjectiles()
    {
        return m_bCanFireProjectiles;
    }

    // vtbl slot 171  PC 0x50F250
    void ZItemTemplateWeapon::SetCanFireProjectiles(bool bCanFireProjectiles)
    {
        m_bCanFireProjectiles = bCanFireProjectiles;
    }

    // vtbl slot 172  PC 0x50F440
    bool ZItemTemplateWeapon::HasSniperMode()
    {
        return m_bSniperMode;
    }

    // vtbl slot 173  PC 0x64A900
    int ZItemTemplateWeapon::GetWeaponType()
    {
        return static_cast<int>(m_eWeaponType);
    }

    // vtbl slot 174  PC 0x6D1340
    EWeaponOperation ZItemTemplateWeapon::GetWeaponOperations()
    {
        return static_cast<EWeaponOperation>(m_WeaponOperations.GetBitfield());
    }

    // vtbl slot 175  PC 0x50F470
    float ZItemTemplateWeapon::GetTimeBetweenShots()
    {
        return m_fTimeBetweenShots;
    }

    // vtbl slot 176  PC 0x50F480
    ZItemTemplateAmmo* ZItemTemplateWeapon::GetAmmoTemplate(int ammoIndex)
    {
        return static_cast<ZItemTemplateAmmo*>(ZGEOM::RefToPtr(m_AmmoTemplate.GetRefNr(ammoIndex)));
    }

    // vtbl slot 177  PC 0x50F530
    float ZItemTemplateWeapon::GetMuzzleVelocity()
    {
        return m_fMuzzleVelocity;
    }

    // vtbl slot 178  PC 0x50F540
    float ZItemTemplateWeapon::GetNearRange()
    {
        return m_fNearRange;
    }

    // vtbl slot 179  PC 0x50F550
    float ZItemTemplateWeapon::GetFarRange()
    {
        return m_fFarRange;
    }

    // vtbl slot 180  PC 0x50F6B0
    float ZItemTemplateWeapon::GetPrecisionDegrees()
    {
        return m_fPrecisionDeg;
    }

    // vtbl slot 181  PC 0x50F4A0
    int ZItemTemplateWeapon::GetDefaultProjectilesPerMagazine()
    {
        ZItemTemplateAmmo* pAmmo = GetAmmoTemplate(0);
        if (pAmmo == nullptr)
            return 0;

        return pAmmo->GetDefaultProjectilesPerMagazine();
    }

    // vtbl slot 182  PC 0x50F260
    float ZItemTemplateWeapon::GetCartridgeSpeed()
    {
        return m_fCartridgeSpeed;
    }

    // vtbl slot 183  PC 0x511F00
    WEAPONOPERATION ZItemTemplateWeapon::SelectNextWeaponOperation(WEAPONOPERATION weaponOperation)
    {
        EWeaponOperation eResult = weaponOperation;

        if (GetWeaponOperations() != static_cast<EWeaponOperation>(0))
        {
            const uint32_t lWeaponOperations = m_WeaponOperations.GetBitfield();

            do
            {
                switch (eResult)
                {
                case EWeaponOperation::WO_MANUAL:    eResult = EWeaponOperation::WO_SEMIAUTO;  break;
                case EWeaponOperation::WO_SEMIAUTO:  eResult = EWeaponOperation::WO_FULLAUTO3; break;
                case EWeaponOperation::WO_FULLAUTO:  eResult = EWeaponOperation::WO_MANUAL;    break;
                case EWeaponOperation::WO_FULLAUTO3: eResult = EWeaponOperation::WO_FULLAUTO;  break;
                default:                                                                     break;
                }
            } while ((static_cast<uint32_t>(eResult) & lWeaponOperations) != static_cast<uint32_t>(eResult));
        }

        return eResult;
    }

    // vtbl slot 184  PC 0x50F570
    double ZItemTemplateWeapon::CalcDamage(const ZItemWeapon* item, float distance)
    {
        // ZItemWeapon::GetAmmoTemplate() is not const; the value is only read here.
        ZItemTemplateAmmo* pAmmo = const_cast<ZItemWeapon*>(item)->GetAmmoTemplate();
        if (pAmmo == nullptr)
            return 0.0;

        if (distance <= m_fNearRange)
            return pAmmo->GetNearDamage() * m_fDamageMultiplier;

        if (distance > m_fFarRange)
            return pAmmo->GetFarDamage() * m_fDamageMultiplier;

        const float fNearDamage = pAmmo->GetNearDamage();
        const float fFarDamage = pAmmo->GetFarDamage();

        return (fNearDamage - (fNearDamage - fFarDamage) *
            ((distance - m_fNearRange) / (m_fFarRange - m_fNearRange))) * m_fDamageMultiplier;
    }

    // vtbl slot 185  PC 0x50F640
    double ZItemTemplateWeapon::CalcImpact(const ZItemWeapon* /*item*/, float distance)
    {
        if (distance <= m_fNearRange)
            return 1.0f * m_fImpact;

        if (distance <= m_fFarRange)
            return (1.0f - (distance - m_fNearRange) / (m_fFarRange - m_fNearRange)) * m_fImpact;

        return 0.0f * m_fImpact;
    }

    // vtbl slot 186  PC 0x50F270
    REFTAB* ZItemTemplateWeapon::GetWeaponParts()
    {
        return &m_WeaponParts;
    }

    // vtbl slot 187  PC 0x511F80
    ZGEOM* ZItemTemplateWeapon::GetMuzzleFire()
    {
        ZGEOM* pMuzzleEffect = ZGEOM::RefToPtr(m_rWeaponFlash);
        if (pMuzzleEffect != nullptr && (pMuzzleEffect->GetObjectId() & ZGROUP::m_Mask) == ZGROUP::m_Id)
            return static_cast<ZGROUP*>(pMuzzleEffect)->FindGeom("MuzzleFire", nullptr);

        return nullptr;
    }

    // vtbl slot 188  PC 0x511FD0
    ZGEOM* ZItemTemplateWeapon::GetMuzzleSmoke()
    {
        ZGEOM* pMuzzleEffect = ZGEOM::RefToPtr(m_rWeaponFlash);
        if (pMuzzleEffect != nullptr && (pMuzzleEffect->GetObjectId() & ZGROUP::m_Mask) == ZGROUP::m_Id)
            return static_cast<ZGROUP*>(pMuzzleEffect)->FindGeom("Smoke", nullptr);

        return nullptr;
    }

    // vtbl slot 189  PC 0x512020
    ZGEOM* ZItemTemplateWeapon::GetMuzzleLight()
    {
        ZGEOM* pMuzzleEffect = ZGEOM::RefToPtr(m_rWeaponFlash);
        if (pMuzzleEffect != nullptr && (pMuzzleEffect->GetObjectId() & ZGROUP::m_Mask) == ZGROUP::m_Id)
            return static_cast<ZGROUP*>(pMuzzleEffect)->FindGeom("Omni01", nullptr);

        return nullptr;
    }

    // vtbl slot 190  PC 0x511FD0 (shares the implementation with GetMuzzleSmoke)
    ZGEOM* ZItemTemplateWeapon::GetCartridge()
    {
        ZGEOM* pMuzzleEffect = ZGEOM::RefToPtr(m_rWeaponFlash);
        if (pMuzzleEffect != nullptr && (pMuzzleEffect->GetObjectId() & ZGROUP::m_Mask) == ZGROUP::m_Id)
            return static_cast<ZGROUP*>(pMuzzleEffect)->FindGeom("Smoke", nullptr);

        return nullptr;
    }

    // vtbl slot 191  PC 0x50F560
    ZGEOM* ZItemTemplateWeapon::GetMuzzleEffect()
    {
        return ZGEOM::RefToPtr(m_rWeaponFlash);
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        // WEAPONOPERATION: the original RTTI also declares this enum (used as a bitfield elsewhere).
        static ZEnumEntry WeaponOperationEntries[] = {
            {nullptr, static_cast<int>(EWeaponOperation::WO_MANUAL), "WO_MANUAL"},
            {&WeaponOperationEntries[0], static_cast<int>(EWeaponOperation::WO_SEMIAUTO), "WO_SEMIAUTO"},
            {&WeaponOperationEntries[1], static_cast<int>(EWeaponOperation::WO_FULLAUTO), "WO_FULLAUTO"},
            {&WeaponOperationEntries[2], static_cast<int>(EWeaponOperation::WO_FULLAUTO3), "WO_FULLAUTO3"},
            {&WeaponOperationEntries[3], 0x7FFFFFFF, "WO_FORCE32"}};
        [[maybe_unused]] static ZEnumInfo WeaponOperationInfo{&WeaponOperationEntries[4], "WEAPONOPERATION", sizeof(WEAPONOPERATION)};

        static ZEnumEntry WeaponTypeEntries[] = {
            {nullptr, WT_PISTOL, "WT_PISTOL"},
            {&WeaponTypeEntries[0], WT_REVOLVER, "WT_REVOLVER"},
            {&WeaponTypeEntries[1], WT_SUBMACHINEGUN, "WT_SUBMACHINEGUN"},
            {&WeaponTypeEntries[2], WT_MACHINEGUN, "WT_MACHINEGUN"},
            {&WeaponTypeEntries[3], WT_RIFLE, "WT_RIFLE"},
            {&WeaponTypeEntries[4], WT_PUMPGUN, "WT_PUMPGUN"},
            {&WeaponTypeEntries[5], WT_SHOTGUN, "WT_SHOTGUN"},
            {&WeaponTypeEntries[6], WT_ROCKETLAUNCHER, "WT_ROCKETLAUNCHER"},
            {&WeaponTypeEntries[7], WT_KNIFE, "WT_KNIFE"},
            {&WeaponTypeEntries[8], WT_PIANO, "WT_PIANO"},
            {&WeaponTypeEntries[9], WT_OTHER, "WT_OTHER"},
            {&WeaponTypeEntries[10], WT_GRENADE, "WT_GRENADE"},
            {&WeaponTypeEntries[11], WT_MOLOTOV, "WT_MOLOTOV"},
            {&WeaponTypeEntries[12], WT_CLOSECOMBAT, "WT_CLOSECOMBAT"},
            {&WeaponTypeEntries[13], WT_FORCE32, "WT_FORCE32"}};
        static ZEnumInfo WeaponTypeInfo{&WeaponTypeEntries[14], "WEAPONTYPE", sizeof(WEAPONTYPE)};

        // Chain is declared tail-first; head (m_SoundDef) is passed to DECLARE_GEOM_CLASS_IMPL.
        static RTP::ZDataProperty<float> CartridgeSpeed{
            .m_Node = {.m_Next = nullptr, .m_Name = "m_fCartridgeSpeed", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Data_float,
            .m_Offset = CLASS_PROPERTY(ZItemTemplateWeapon, m_fCartridgeSpeed)};

        static RTP::ZDataProperty<uint> NumInstances{
            .m_Node = {.m_Next = CartridgeSpeed, .m_Name = "m_lNumInstances", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Data_uint,
            .m_Offset = CLASS_PROPERTY(ZItemTemplateWeapon, m_lNumInstances)};

        static RTP::ZDataProperty<ZREF> WeaponFlash{
            .m_Node = {.m_Next = NumInstances, .m_Name = "m_rWeaponFlash", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Data_uint,
            .m_Offset = CLASS_PROPERTY(ZItemTemplateWeapon, m_rWeaponFlash)};

        static RTP::ZDataProperty<REFTAB> WeaponParts{
            .m_Node = {.m_Next = WeaponFlash, .m_Name = "m_WeaponParts", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Data_REFTAB,
            .m_Offset = CLASS_PROPERTY(ZItemTemplateWeapon, m_WeaponParts)};

        static RTP::ZDataProperty<bool> SniperMode{
            .m_Node = {.m_Next = WeaponParts, .m_Name = "m_bSniperMode", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Data_bool,
            .m_Offset = CLASS_PROPERTY(ZItemTemplateWeapon, m_bSniperMode)};

        static RTP::ZDataProperty<char> CanHaveMagazines{
            .m_Node = {.m_Next = SniperMode, .m_Name = "m_bCanHaveMagazines", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Data_char,
            .m_Offset = CLASS_PROPERTY(ZItemTemplateWeapon, m_bCanHaveMagazines)};

        static RTP::ZDataProperty<bool> CanFireProjectiles{
            .m_Node = {.m_Next = CanHaveMagazines, .m_Name = "m_bCanFireProjectiles", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Data_bool,
            .m_Offset = CLASS_PROPERTY(ZItemTemplateWeapon, m_bCanFireProjectiles)};

        static RTP::ZDataProperty<float> TimeBetweenShots{
            .m_Node = {.m_Next = CanFireProjectiles, .m_Name = "m_fTimeBetweenShots", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Data_float,
            .m_Offset = CLASS_PROPERTY(ZItemTemplateWeapon, m_fTimeBetweenShots)};

        static RTP::ZDataProperty<float> Impact{
            .m_Node = {.m_Next = TimeBetweenShots, .m_Name = "m_fImpact", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Data_float,
            .m_Offset = CLASS_PROPERTY(ZItemTemplateWeapon, m_fImpact)};

        static RTP::ZDataProperty<float> DamageMultiplier{
            .m_Node = {.m_Next = Impact, .m_Name = "m_fDamageMultiplier", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Data_float,
            .m_Offset = CLASS_PROPERTY(ZItemTemplateWeapon, m_fDamageMultiplier)};

        static RTP::ZDataProperty<float> FarRange{
            .m_Node = {.m_Next = DamageMultiplier, .m_Name = "m_fFarRange", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Data_float,
            .m_Offset = CLASS_PROPERTY(ZItemTemplateWeapon, m_fFarRange)};

        static RTP::ZDataProperty<float> NearRange{
            .m_Node = {.m_Next = FarRange, .m_Name = "m_fNearRange", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Data_float,
            .m_Offset = CLASS_PROPERTY(ZItemTemplateWeapon, m_fNearRange)};

        static RTP::ZDataProperty<float> PrecisionDeg{
            .m_Node = {.m_Next = NearRange, .m_Name = "m_fPrecisionDeg", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Data_float,
            .m_Offset = CLASS_PROPERTY(ZItemTemplateWeapon, m_fPrecisionDeg)};

        static RTP::ZDataProperty<float> MuzzleVelocity{
            .m_Node = {.m_Next = PrecisionDeg, .m_Name = "m_fMuzzleVelocity", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Data_float,
            .m_Offset = CLASS_PROPERTY(ZItemTemplateWeapon, m_fMuzzleVelocity)};

        static RTP::ZDataProperty<REFTAB> AmmoTemplate{
            .m_Node = {.m_Next = MuzzleVelocity, .m_Name = "m_AmmoTemplate", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Data_REFTAB,
            .m_Offset = CLASS_PROPERTY(ZItemTemplateWeapon, m_AmmoTemplate)};

        static RTP::ZEnumProperty WeaponType{
            .m_Node = {.m_Next = AmmoTemplate, .m_Name = "m_eWeaponType", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Enum,
            .m_Offset = CLASS_PROPERTY(ZItemTemplateWeapon, m_eWeaponType),
            .m_Info = &WeaponTypeInfo};

        static RTP::ZDataProperty<ZBitfield<EWeaponOperation>> WeaponOperations{
            .m_Node = {.m_Next = WeaponType, .m_Name = "m_WeaponOperations", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Data_ZBitfield_WEAPONOPERATION,
            .m_Offset = CLASS_PROPERTY(ZItemTemplateWeapon, m_WeaponOperations)};

        static RTP::ZDataProperty<ZSDOwner> SoundDef{
            .m_Node = {.m_Next = WeaponOperations, .m_Name = "m_SoundDef", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Data_ZSDOwner,
            .m_Offset = CLASS_PROPERTY(ZItemTemplateWeapon, m_SoundDef)};
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZItemTemplateWeapon, // ClassName
        ZItemTemplate, // BaseClass
        0x0099C028, // OldClassInfoAddr
        "ZItemTemplateWeapon", // FactoryName
        0x00773EA0, // FactoryNameAddr
        cProperties::SoundDef, // FirstProperty
        0x0080C698, // PropertiesAddr
        0x0099BF38, // IdAddr
        0x0099BF3C // MaskAddr
    );
#   pragma endregion
}
