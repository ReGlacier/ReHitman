#include <BloodMoney/Game/ZHM3WeaponUpgradeGeom.h>

#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/RTP/VirtualTables.h>


namespace Hitman
{
    // PC 0x653110. Only chains ZSTDOBJ and installs the vftable; the PC ctor does not initialise
    // m_eType (it is filled from the RTP "m_eType" property on load).
    ZHM3WeaponUpgradeGeom::ZHM3WeaponUpgradeGeom(const char* psName, Glacier::ZBaseGeom* pBaseGeom)
        : Glacier::ZSTDOBJ(psName, pBaseGeom)
        , m_eType(UT_Dummy)
    {
    }

    // vtbl slot 0 (deleting destructor)
    ZHM3WeaponUpgradeGeom::~ZHM3WeaponUpgradeGeom() = default;

    // vtbl slot 12 (RTTI)  PC 0x6527E0
    const Glacier::RTP::ZPropertyInfo& ZHM3WeaponUpgradeGeom::GetProperties() const
    {
        return ZHM3WeaponUpgradeGeom::Info;
    }

    // vtbl slot 13 (RTTI)  PC 0x6531D0
    uint32_t ZHM3WeaponUpgradeGeom::GetObjectId() const
    {
        return ZHM3WeaponUpgradeGeom::m_Id;
    }

    // vtbl slot 14 (RTTI)  PC 0x6531E0
    void ZHM3WeaponUpgradeGeom::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZHM3WeaponUpgradeGeom::m_Id;
        mask = ZHM3WeaponUpgradeGeom::m_Mask;
    }

    // vtbl slot 15 (RTTI)  PC 0x6527F0
    Glacier::ZGEOMCLASSINFO* ZHM3WeaponUpgradeGeom::GetOldClassInfo() const
    {
        return ZHM3WeaponUpgradeGeom::m_OldClassInfo;
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        // "EUpgradeType" (PC 0x808F7C), the type carried by this geom.
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

        // Game offset 0x0C -> class offset +0x10 (ZHM3WeaponUpgradeGeom::Info.First is 0x809130).
        static Glacier::RTP::ZEnumProperty Type{
            .m_Node = {.m_Next = nullptr, .m_Name = "m_eType", .m_Filter = 1},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Enum,
            .m_Offset = CLASS_PROPERTY(ZHM3WeaponUpgradeGeom, m_eType),
            .m_Info = &UpgradeTypeInfo};
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZHM3WeaponUpgradeGeom,   // ClassName
        Glacier::ZSTDOBJ,        // BaseClass
        0x0,                     // OldClassInfoAddr (documentation only; not bound by the macro)
        "ZHM3WeaponUpgradeGeom", // FactoryName
        0x0,                     // FactoryNameAddr (documentation only; not bound by the macro)
        cProperties::Type,       // FirstProperty
        0x00809148,              // PropertiesAddr (ZHM3WeaponUpgradeGeom::Info)
        0x0,                     // IdAddr (documentation only; not bound by the macro)
        0x0                      // MaskAddr (documentation only; not bound by the macro)
    );
#   pragma endregion
}
