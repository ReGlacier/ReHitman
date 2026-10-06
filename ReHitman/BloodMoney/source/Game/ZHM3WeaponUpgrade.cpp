#include <BloodMoney/Game/ZHM3WeaponUpgrade.h>

#include <BloodMoney/Game/Items/ZHM3ItemWeaponCustom.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/ZUniAssert.h>


namespace Hitman
{
    // PC 0x653090. The PC ctor only chains ZSTDOBJ and zeroes the four REFTAB pointers
    // (+0x18..+0x24); m_fCost / m_eType / m_eTier are filled from the level data / RTP.
    ZHM3WeaponUpgrade::ZHM3WeaponUpgrade(const char* psName, Glacier::ZBaseGeom* pBaseGeom)
        : Glacier::ZSTDOBJ(psName, pBaseGeom)
        , m_fCost(0.f)
        , m_eType(UT_Dummy)
        , m_pRequires(nullptr)
        , m_pExcludes(nullptr)
        , m_BoneIncludes(nullptr)
        , m_BoneExcludes(nullptr)
        , m_eTier(UPGRADE_TIER1)
    {
    }

    // vtbl slot 0 (deleting destructor)  PC 0x6532B0
    ZHM3WeaponUpgrade::~ZHM3WeaponUpgrade() = default;

    // vtbl slot 12 (RTTI)  PC 0x6528B0
    const Glacier::RTP::ZPropertyInfo& ZHM3WeaponUpgrade::GetProperties() const
    {
        return ZHM3WeaponUpgrade::Info;
    }

    // vtbl slot 13 (RTTI)  PC 0x6531A0
    uint32_t ZHM3WeaponUpgrade::GetObjectId() const
    {
        return ZHM3WeaponUpgrade::m_Id;
    }

    // vtbl slot 14 (RTTI)  PC 0x6531B0
    void ZHM3WeaponUpgrade::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZHM3WeaponUpgrade::m_Id;
        mask = ZHM3WeaponUpgrade::m_Mask;
    }

    // vtbl slot 15 (RTTI)  PC 0x6528C0
    Glacier::ZGEOMCLASSINFO* ZHM3WeaponUpgrade::GetOldClassInfo() const
    {
        return ZHM3WeaponUpgrade::m_OldClassInfo;
    }

    // vtbl slot 104  PC 0x652960. The PC body copies the two scalar members only.
    void ZHM3WeaponUpgrade::CopyData(const Glacier::ZGEOM* Source)
    {
        const ZHM3WeaponUpgrade* pSource = static_cast<const ZHM3WeaponUpgrade*>(Source);
        m_fCost = pSource->m_fCost;
        m_eType = pSource->m_eType;
    }

    // PC 0x652980. When bApply is set the upgrade broadcasts the "ApplyWeaponUpgrade" command to
    // itself with the weapon as payload; the attached ZHM3WeaponUpgradeProperty events then read
    // and write the weapon's stats. The PC body also refreshes the custom template afterwards.
    void ZHM3WeaponUpgrade::ApplyUpgrade(ZHM3ItemWeaponCustom* pWeapon, bool bApply)
    {
        if (bApply)
        {
            ZASSERT(static_cast<bool>(m_msgApplyUpgrade));
            SendCommand(static_cast<Glacier::ZMSGID>(m_msgApplyUpgrade), pWeapon, nullptr);
        }
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        // "EUpgradeType" (PC 0x808F7C / "RTP::Type::Enum::ZHM3WeaponUpgrade::EUpgradeType").
        static Glacier::ZEnumEntry UpgradeTypeEntries[] = {
            {nullptr, UT_Dummy, "UT_Dummy"},
            {&UpgradeTypeEntries[0], UT_AmmoACP, "UT_AmmoACP"},
            {&UpgradeTypeEntries[1], UT_AmmoArmorPiercing, "UT_AmmoArmorPiercing"},
            {&UpgradeTypeEntries[2], UT_Ammo127mm, "UT_Ammo127mm"},
            {&UpgradeTypeEntries[3], UT_AmmoMagnum, "UT_AmmoMagnum"},
            {&UpgradeTypeEntries[4], UT_AmmoFlechetteSlugs, "UT_AmmoFlechetteSlugs"},
            {&UpgradeTypeEntries[5], UT_AmmoGaugesSlugs, "UT_AmmoGaugesSlugs"},
            {&UpgradeTypeEntries[6], UT_AmmoLowVelocity, "UT_AmmoLowVelocity"},
            {&UpgradeTypeEntries[7], UT_Magazine, "UT_Magazine"},
            {&UpgradeTypeEntries[8], UT_Silencer1, "UT_Silencer1"},
            {&UpgradeTypeEntries[9], UT_Silencer2, "UT_Silencer2"},
            {&UpgradeTypeEntries[10], UT_LaserSight, "UT_LaserSight"},
            {&UpgradeTypeEntries[11], UT_DualAction, "UT_DualAction"},
            {&UpgradeTypeEntries[12], UT_DualActionAuto, "UT_DualActionAuto"},
            {&UpgradeTypeEntries[13], UT_DoubleCapMag, "UT_DoubleCapMag"},
            {&UpgradeTypeEntries[14], UT_FullAuto, "UT_FullAuto"},
            {&UpgradeTypeEntries[15], UT_ReloadBoost, "UT_ReloadBoost"},
            {&UpgradeTypeEntries[16], UT_BeltFeeding, "UT_BeltFeeding"},
            {&UpgradeTypeEntries[17], UT_BiPod, "UT_BiPod"},
            {&UpgradeTypeEntries[18], UT_ScopeType1, "UT_ScopeType1"},
            {&UpgradeTypeEntries[19], UT_ScopeType2, "UT_ScopeType2"},
            {&UpgradeTypeEntries[20], UT_ScopeType3, "UT_ScopeType3"},
            {&UpgradeTypeEntries[21], UT_NightVision, "UT_NightVision"},
            {&UpgradeTypeEntries[22], UT_Lightweight, "UT_Lightweight"},
            {&UpgradeTypeEntries[23], UT_DefaultAmmo, "UT_DefaultAmmo"},
            {&UpgradeTypeEntries[24], UT_DefaultNoScope, "UT_DefaultNoScope"},
            {&UpgradeTypeEntries[25], UT_DefaultBarrel, "UT_DefaultBarrel"},
            {&UpgradeTypeEntries[26], UT_DefaultMagazine, "UT_DefaultMagazine"},
            {&UpgradeTypeEntries[27], UT_RailMount, "UT_RailMount"},
            {&UpgradeTypeEntries[28], UT_CarbonBarrel, "UT_CarbonBarrel"},
            {&UpgradeTypeEntries[29], UT_Buttstock, "UT_Buttstock"},
            {&UpgradeTypeEntries[30], UT_Suitcase, "UT_Suitcase"},
            {&UpgradeTypeEntries[31], UT_DefaultButtStock, "UT_DefaultButtStock"},
            {&UpgradeTypeEntries[32], UT_DefaultGrip, "UT_DefaultGrip"},
            {&UpgradeTypeEntries[33], UT_DefaultHandguard, "UT_DefaultHandguard"},
            {&UpgradeTypeEntries[34], UT_DefaultHandle, "UT_DefaultHandle"},
            {&UpgradeTypeEntries[35], UT_DefaultSight, "UT_DefaultSight"},
            {&UpgradeTypeEntries[36], UT_BoltAction, "UT_BoltAction"},
            {&UpgradeTypeEntries[37], UT_RedDotSight, "UT_RedDotSight"},
            {&UpgradeTypeEntries[38], UT_ShortBarrel, "UT_ShortBarrel"},
            {&UpgradeTypeEntries[39], UT_PistolGrip, "UT_PistolGrip"},
            {&UpgradeTypeEntries[40], UT_HandGuard, "UT_HandGuard"},
            {&UpgradeTypeEntries[41], UT_RapidFire, "UT_RapidFire"},
            {&UpgradeTypeEntries[42], UT_LongSlide, "UT_LongSlide"},
            {&UpgradeTypeEntries[43], UT_ClipX2, "UT_ClipX2"},
            {&UpgradeTypeEntries[44], UT_ClipX3, "UT_ClipX3"},
            {&UpgradeTypeEntries[45], UT_ClipX4, "UT_ClipX4"},
            {&UpgradeTypeEntries[46], UT_NumUpgradeTypes, "UT_NumUpgradeTypes"}};
        static Glacier::ZEnumInfo UpgradeTypeInfo{&UpgradeTypeEntries[47], "EUpgradeType", sizeof(EUpgradeType)};

        // "EUpgradeTier" (PC 0x808CC8).
        static Glacier::ZEnumEntry UpgradeTierEntries[] = {
            {nullptr, UPGRADE_TIER1, "UPGRADE_TIER1"},
            {&UpgradeTierEntries[0], UPGRADE_TIER2, "UPGRADE_TIER2"},
            {&UpgradeTierEntries[1], UPGRADE_TIER3, "UPGRADE_TIER3"},
            {&UpgradeTierEntries[2], UPGRADE_TIER4, "UPGRADE_TIER4"},
            {&UpgradeTierEntries[3], UPGRADE_TIER5, "UPGRADE_TIER5"}};
        static Glacier::ZEnumInfo UpgradeTierInfo{&UpgradeTierEntries[4], "EUpgradeTier", sizeof(EUpgradeTier)};

        // The PC chain is laid out tail-first in memory; the head ("m_fCost", PC 0x808FA0) is the
        // FirstProperty passed to DECLARE_GEOM_CLASS_IMPL. Game offset -> class offset = +4
        // (ZHM3WeaponUpgrade::Info.First is 0x808FA0).
        static Glacier::RTP::ZEnumProperty UpgradeTier{
            .m_Node = {.m_Next = nullptr, .m_Name = "m_eTier", .m_Filter = 1},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Enum,
            .m_Offset = CLASS_PROPERTY(ZHM3WeaponUpgrade, m_eTier),
            .m_Info = &UpgradeTierInfo};

        static Glacier::RTP::ZDataProperty<Glacier::REFTAB*> BoneExcludes{
            .m_Node = {.m_Next = UpgradeTier, .m_Name = "m_BoneExcludes", .m_Filter = 1},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_REFTAB_ptr,
            .m_Offset = CLASS_PROPERTY(ZHM3WeaponUpgrade, m_BoneExcludes)};

        static Glacier::RTP::ZDataProperty<Glacier::REFTAB*> BoneIncludes{
            .m_Node = {.m_Next = BoneExcludes, .m_Name = "m_BoneIncludes", .m_Filter = 1},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_REFTAB_ptr,
            .m_Offset = CLASS_PROPERTY(ZHM3WeaponUpgrade, m_BoneIncludes)};

        static Glacier::RTP::ZDataProperty<Glacier::REFTAB*> Excludes{
            .m_Node = {.m_Next = BoneIncludes, .m_Name = "m_pExcludes", .m_Filter = 1},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_REFTAB_ptr,
            .m_Offset = CLASS_PROPERTY(ZHM3WeaponUpgrade, m_pExcludes)};

        static Glacier::RTP::ZDataProperty<Glacier::REFTAB*> Requires{
            .m_Node = {.m_Next = Excludes, .m_Name = "m_pRequires", .m_Filter = 1},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_REFTAB_ptr,
            .m_Offset = CLASS_PROPERTY(ZHM3WeaponUpgrade, m_pRequires)};

        static Glacier::RTP::ZEnumProperty UpgradeType{
            .m_Node = {.m_Next = Requires, .m_Name = "m_eType", .m_Filter = 1},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Enum,
            .m_Offset = CLASS_PROPERTY(ZHM3WeaponUpgrade, m_eType),
            .m_Info = &UpgradeTypeInfo};

        static Glacier::RTP::ZDataProperty<float> Cost{
            .m_Node = {.m_Next = UpgradeType, .m_Name = "m_fCost", .m_Filter = 1},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_float,
            .m_Offset = CLASS_PROPERTY(ZHM3WeaponUpgrade, m_fCost)};
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZHM3WeaponUpgrade,        // ClassName
        Glacier::ZSTDOBJ,         // BaseClass
        0x009B1D10,               // OldClassInfoAddr
        "ZHM3WeaponUpgrade",      // FactoryName
        0x0,                      // FactoryNameAddr (documentation only; not bound by the macro)
        cProperties::Cost,        // FirstProperty
        0x00808FB4,               // PropertiesAddr (ZHM3WeaponUpgrade::Info)
        0x009B1C68,               // IdAddr (ZHM3WeaponUpgrade::m_Id)
        0x009B1C6C                // MaskAddr (ZHM3WeaponUpgrade::m_Mask)
    );

    STATIC_CLASS_VAR_IMPL(ZHM3WeaponUpgrade, Glacier::ZMessageResolver, m_msgApplyUpgrade, 0x009B1E20, {"ApplyWeaponUpgrade"});
#   pragma endregion
}
