#include <Glacier/Items/ZItemTemplateAmmo.h>

#include <Glacier/Data/ZGameData.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/Items/ZItemAmmo.h>
#include <Glacier/Items/EDamageType.h>
#include <Glacier/RTP/Base.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/ZUniMemory.h>

#include <cstring>


namespace Glacier
{
    // vtbl slot 82  PS2 0x279BF8 / iOS 0x10038BF8C
    ZItemTemplateAmmo::ZItemTemplateAmmo(const char* psName, ZBaseGeom* pBaseGeom)
        : ZItemTemplate(psName, pBaseGeom)
    {
        m_lProjectilesPerMagazine = 0;
        m_lProjectilesPerShot = 1;
        m_fNearDamage = 0.0f;
        m_fFarDamage = 0.0f;
        m_bCanSplashDamage = false;
        m_bUseImpactSound = true;
        m_MaterialEnumId = 0;
        m_rProjectile = 0;
        m_rCartridge = 0;
        m_eDamageType = EDamageType::LETHAL;
        m_pProjectileList = nullptr;
        m_lProjectileListSize = 10;
        m_lProjectileCurrent = 0;
    }

    ZItemTemplateAmmo::~ZItemTemplateAmmo() = default;

    // vtbl slot 12  PC 0x50FEE0
    const RTP::ZPropertyInfo& ZItemTemplateAmmo::GetProperties() const
    {
        return ZItemTemplateAmmo::Info;
    }

    // vtbl slot 13  PC 0x510AE0
    uint32_t ZItemTemplateAmmo::GetObjectId() const
    {
        return ZItemTemplateAmmo::m_Id;
    }

    // vtbl slot 14  PC 0x510AF0
    void ZItemTemplateAmmo::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZItemTemplateAmmo::m_Id;
        mask = ZItemTemplateAmmo::m_Mask;
    }

    // vtbl slot 15  PC 0x50FEF0
    ZGEOMCLASSINFO* ZItemTemplateAmmo::GetOldClassInfo() const
    {
        return ZItemTemplateAmmo::m_OldClassInfo;
    }

    // vtbl slot 82  PC 0x512850 / PS2 0x27A040
    void ZItemTemplateAmmo::ClassInit()
    {
        // Resolve the ammo material enum id from the engine's material-description database.
        const char* psName = Name();
        if (psName == nullptr)
            psName = "<NONAME>";

        m_MaterialEnumId = g_pGameData->GetAmmoEnumId(psName).m_Value;
    }

    // PS2 0x27A0C4
    void ZItemTemplateAmmo::ClassInit2()
    {
    }

    // PC 0x50FF30 / PS2 0x27A0E8 / iOS 0x10038C1B4
    void ZItemTemplateAmmo::PostClassInit()
    {
        ZGEOM::PostClassInit();

        if (m_pProjectileList == nullptr)
        {
            m_pProjectileList = static_cast<ZREF*>(
                ZUniMemory::Allocate(static_cast<int>(sizeof(ZREF) * m_lProjectileListSize)));
            memset(m_pProjectileList, 0, sizeof(ZREF) * m_lProjectileListSize);

            // Instantiate one projectile geom per list slot, parented to the scene root, and
            // remember its ref. The template's projectile is resolved on every iteration; an
            // absent projectile ends the loop early (the remaining slots stay null).
            for (uint32_t i = 0; i < m_lProjectileListSize; ++i)
            {
                ZGEOM* pProjectile = GetProjectile();
                if (pProjectile == nullptr)
                    break;

                ZGEOM* pInstance = pProjectile->DuplicateInit(
                    g_pEngineData->m_pRoot, nullptr, nullptr, nullptr, true);
                m_pProjectileList[i] = pInstance->GetRef();
            }
        }
    }

    // vtbl slot 110  PS2 0x27A260 / iOS 0x10038C274
    void ZItemTemplateAmmo::LoadSave(ISerializerStream& stream, bool bSave)
    {
        ZGROUP::LoadSave(stream, bSave);

        stream.Exchange("ProjectileListSize", m_lProjectileListSize);

        if (bSave)
        {
            ZASSERT(m_pProjectileList != nullptr);
        }
        else if (m_pProjectileList == nullptr)
        {
            m_pProjectileList = static_cast<ZREF*>(
                ZUniMemory::Allocate(static_cast<int>(sizeof(ZREF) * m_lProjectileListSize)));
            memset(m_pProjectileList, 0, sizeof(ZREF) * m_lProjectileListSize);
        }

        for (uint32_t i = 0; i < m_lProjectileListSize; ++i)
            stream.Exchange("ProjectileList[i]", m_pProjectileList[i]);
    }

    // PS2 0x27A52C
    void ZItemTemplateAmmo::SetStates(CCom* pCom)
    {
        ZItemTemplate::SetStates(pCom);
    }

    // vtbl slot 104  PS2 0x279F48 / iOS 0x10038C0E8
    void ZItemTemplateAmmo::CopyData(const ZGEOM* Source)
    {
        ZGROUP::CopyData(Source);

        if (!Source->IsDerivedFrom<ZItemTemplateAmmo>())
            return;

        const ZItemTemplateAmmo* pSource = static_cast<const ZItemTemplateAmmo*>(Source);

        m_lProjectilesPerMagazine = pSource->m_lProjectilesPerMagazine;
        m_lProjectilesPerShot = pSource->m_lProjectilesPerShot;
        m_fNearDamage = pSource->m_fNearDamage;
        m_fFarDamage = pSource->m_fFarDamage;
        m_bCanSplashDamage = pSource->m_bCanSplashDamage;
        m_bUseImpactSound = pSource->m_bUseImpactSound;
        m_rProjectile = pSource->m_rProjectile;
        m_rCartridge = pSource->m_rCartridge;
        m_eDamageType = pSource->m_eDamageType;
    }

    // vtbl slot 149  PS2 0x73838C
    uint32_t ZItemTemplateAmmo::GetItemClassId() const
    {
        return ZItemAmmo::m_TypeId;
    }

    // PC 0x50FF00 (GetDefaultProjectilesPerMagazine)
    int ZItemTemplateAmmo::GetDefaultProjectilesPerMagazine()
    {
        return m_lProjectilesPerMagazine;
    }

    // PC 0x50FF10
    int ZItemTemplateAmmo::GetProjectilesPerShot()
    {
        return m_lProjectilesPerShot;
    }

    // PC 0x50FFF0
    float ZItemTemplateAmmo::GetNearDamage()
    {
        return m_fNearDamage;
    }

    // PC 0x510000
    float ZItemTemplateAmmo::GetFarDamage()
    {
        return m_fFarDamage;
    }

    // PC 0x50FF20
    void ZItemTemplateAmmo::GetMaterialEnumId(int* pRes)
    {
        *pRes = m_MaterialEnumId;
    }

    // PC 0x510020
    float ZItemTemplateAmmo::GetSplashDamage()
    {
        return m_bCanSplashDamage;
    }

    // PS2 0x27A880
    bool ZItemTemplateAmmo::GetCanPenetrate()
    {
        return false;
    }

    // PC 0x510040
    ZGEOM* ZItemTemplateAmmo::GetProjectile()
    {
        return ZGEOM::RefToPtr(m_rProjectile);
    }

    // PS2 0x27A83C
    ZGEOM* ZItemTemplateAmmo::GetCartridge()
    {
        return ZGEOM::RefToPtr(m_rCartridge);
    }

    // PC 0x514F10 / PS2 0x27A5B0
    ZGEOM* ZItemTemplateAmmo::GetProjectileInstance()
    {
        ZASSERT(m_pProjectileList != nullptr);
        if (m_pProjectileList == nullptr)
            return nullptr;

        const ZREF projectileRef = m_pProjectileList[m_lProjectileCurrent];

        if (++m_lProjectileCurrent == m_lProjectileListSize)
            m_lProjectileCurrent = 0;

        ZGEOM* pResult = ZGEOM::RefToPtr(projectileRef);
        if (pResult != nullptr && (pResult->m_baseGeom->m_lControl & 0x400) != 0)
            pResult->MakeActive();

        return pResult;
    }

    // PC 0x477740
    EDamageType ZItemTemplateAmmo::GetDamageType()
    {
        return m_eDamageType;
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        static ZEnumEntry DamageTypeEntries[] = {
            {nullptr, static_cast<int>(EDamageType::LETHAL), "LETHAL"},
            {&DamageTypeEntries[0], static_cast<int>(EDamageType::TRANQUILIZING), "TRANQUILIZING"}};
        static ZEnumInfo DamageTypeInfo{&DamageTypeEntries[1], "eDamageType", sizeof(EDamageType)};

        // Chain is declared tail-first; head (m_lProjectilesPerMagazine) is passed to
        // DECLARE_GEOM_CLASS_IMPL.
        static RTP::ZDataProperty<uint> ProjectileCurrent{
            .m_Node = {.m_Next = nullptr, .m_Name = "m_lProjectileCurrent", .m_Filter = 2},
            .m_VirtualTable = &RTP::VirtualTables::Data_uint,
            .m_Offset = CLASS_PROPERTY(ZItemTemplateAmmo, m_lProjectileCurrent)};

        static RTP::ZEnumProperty DamageType{
            .m_Node = {.m_Next = ProjectileCurrent, .m_Name = "m_eDamageType", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Enum,
            .m_Offset = CLASS_PROPERTY(ZItemTemplateAmmo, m_eDamageType),
            .m_Info = &DamageTypeInfo};

        static RTP::ZDataProperty<uint> Cartridge{
            .m_Node = {.m_Next = DamageType, .m_Name = "m_rCartridge", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Data_uint,
            .m_Offset = CLASS_PROPERTY(ZItemTemplateAmmo, m_rCartridge)};

        static RTP::ZDataProperty<uint> Projectile{
            .m_Node = {.m_Next = Cartridge, .m_Name = "m_rProjectile", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Data_uint,
            .m_Offset = CLASS_PROPERTY(ZItemTemplateAmmo, m_rProjectile)};

        static RTP::ZDataProperty<bool> UseImpactSound{
            .m_Node = {.m_Next = Projectile, .m_Name = "m_bUseImpactSound", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Data_bool,
            .m_Offset = CLASS_PROPERTY(ZItemTemplateAmmo, m_bUseImpactSound)};

        static RTP::ZDataProperty<bool> SplashDamage{
            .m_Node = {.m_Next = UseImpactSound, .m_Name = "m_bSplashDamage", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Data_bool,
            .m_Offset = CLASS_PROPERTY(ZItemTemplateAmmo, m_bCanSplashDamage)};

        static RTP::ZDataProperty<float> FarDamage{
            .m_Node = {.m_Next = SplashDamage, .m_Name = "m_fFarDamage", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Data_float,
            .m_Offset = CLASS_PROPERTY(ZItemTemplateAmmo, m_fFarDamage)};

        static RTP::ZDataProperty<float> NearDamage{
            .m_Node = {.m_Next = FarDamage, .m_Name = "m_fNearDamage", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Data_float,
            .m_Offset = CLASS_PROPERTY(ZItemTemplateAmmo, m_fNearDamage)};

        static RTP::ZDataProperty<int> ProjectilesPerShot{
            .m_Node = {.m_Next = NearDamage, .m_Name = "m_lProjectilesPerShot", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Data_int,
            .m_Offset = CLASS_PROPERTY(ZItemTemplateAmmo, m_lProjectilesPerShot)};

        static RTP::ZDataProperty<int> ProjectilesPerMagazine{
            .m_Node = {.m_Next = ProjectilesPerShot, .m_Name = "m_lProjectilesPerMagazine", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Data_int,
            .m_Offset = CLASS_PROPERTY(ZItemTemplateAmmo, m_lProjectilesPerMagazine)};
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZItemTemplateAmmo, // ClassName
        ZItemTemplate, // BaseClass
        0x0099C078, // OldClassInfoAddr
        "ZItemTemplateAmmo", // FactoryName
        0x00773EB4, // FactoryNameAddr
        cProperties::ProjectilesPerMagazine, // FirstProperty
        0x0080C8E4, // PropertiesAddr
        0x0099BF40, // IdAddr
        0x0099BF44 // MaskAddr
    );
#   pragma endregion
}
