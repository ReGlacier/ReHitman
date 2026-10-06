#include <BloodMoney/Game/Items/ZHM3ItemTemplateBomb.h>

#include <Glacier/RTP/VirtualTables.h>

namespace Hitman
{
    ZHM3ItemTemplateBomb::ZHM3ItemTemplateBomb(const char* psName, Glacier::ZBaseGeom* pBaseGeom)
        : ZHM3ItemTemplateWeapon(psName, pBaseGeom)
        , m_pEffectGroup(nullptr)
    {
    }

    // vtbl slot 12 (RTTI)  PC 0x64F7B0
    const Glacier::RTP::ZPropertyInfo& ZHM3ItemTemplateBomb::GetProperties() const
    {
        return ZHM3ItemTemplateBomb::Info;
    }

    // vtbl slot 13 (RTTI)  PC 0x64F7E0
    uint32_t ZHM3ItemTemplateBomb::GetObjectId() const
    {
        return ZHM3ItemTemplateBomb::m_Id;
    }

    // vtbl slot 14 (RTTI)  PC 0x64F7F0
    void ZHM3ItemTemplateBomb::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZHM3ItemTemplateBomb::m_Id;
        mask = ZHM3ItemTemplateBomb::m_Mask;
    }

    // vtbl slot 15 (RTTI)  PC 0x64F7C0 -> &ZHM3ItemTemplateBomb::m_OldClassInfo
    Glacier::ZGEOMCLASSINFO* ZHM3ItemTemplateBomb::GetOldClassInfo() const
    {
        return ZHM3ItemTemplateBomb::m_OldClassInfo;
    }

    // vtbl slot 149  PC 0x64F7D0. Differs from the weapon's id (0x10042B) so that item creation
    // spawns a ZHM3ItemBomb rather than a ZHM3ItemWeapon.
    uint32_t ZHM3ItemTemplateBomb::GetItemClassId() const
    {
        return 0x100450u;
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        // The PC chain is laid out tail-first; the head (m_fMaxDamage) is the FirstProperty passed
        // to DECLARE_GEOM_CLASS_IMPL. Game offset -> class offset = +4
        // (ZHM3ItemTemplateBomb::Info.First is 0x80FAF8).
        static Glacier::RTP::ZDataProperty<float> ExplodeTimer{
            .m_Node = {.m_Next = nullptr, .m_Name = "m_fExplodeTimer", .m_Filter = 1},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_float,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemTemplateBomb, m_fExplodeTimer)};

        static Glacier::RTP::ZDataProperty<float> MaxRange{
            .m_Node = {.m_Next = ExplodeTimer, .m_Name = "m_fMaxRange", .m_Filter = 1},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_float,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemTemplateBomb, m_fMaxRange)};

        static Glacier::RTP::ZDataProperty<float> MaxDamageRange{
            .m_Node = {.m_Next = MaxRange, .m_Name = "m_fMaxDamageRange", .m_Filter = 1},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_float,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemTemplateBomb, m_fMaxDamageRange)};

        static Glacier::RTP::ZDataProperty<float> MaxDamage{
            .m_Node = {.m_Next = MaxDamageRange, .m_Name = "m_fMaxDamage", .m_Filter = 1},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_float,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemTemplateBomb, m_fMaxDamage)};
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZHM3ItemTemplateBomb,       // ClassName
        ZHM3ItemTemplateWeapon,     // BaseClass
        0x009B1978,                 // OldClassInfoAddr
        "ZHM3ItemTemplateBomb",     // FactoryName
        0x0,                        // FactoryNameAddr (documentation only; not bound by the macro)
        cProperties::MaxDamage,     // FirstProperty
        0x0080FB0C,                 // PropertiesAddr (ZHM3ItemTemplateBomb::Info)
        0x009B1550,                 // IdAddr (ZHM3ItemTemplateBomb::m_Id)
        0x009B1554                  // MaskAddr (ZHM3ItemTemplateBomb::m_Mask)
    );
#   pragma endregion
}
