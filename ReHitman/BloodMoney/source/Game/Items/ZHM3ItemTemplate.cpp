#include <BloodMoney/Game/Items/ZHM3ItemTemplate.h>

#include <Glacier/RTP/VirtualTables.h>

namespace Hitman
{
    ZHM3ItemTemplate::ZHM3ItemTemplate(const char* psName, Glacier::ZBaseGeom* pBaseGeom)
        : Glacier::ZItemTemplate(psName, pBaseGeom)
    {
    }

    // vtbl slot 12 (RTTI)  PC 0x64D410
    const Glacier::RTP::ZPropertyInfo& ZHM3ItemTemplate::GetProperties() const
    {
        return ZHM3ItemTemplate::Info;
    }

    // vtbl slot 13 (RTTI)  PC 0x64D440
    uint32_t ZHM3ItemTemplate::GetObjectId() const
    {
        return ZHM3ItemTemplate::m_Id;
    }

    // vtbl slot 14 (RTTI)  PC 0x64D450
    void ZHM3ItemTemplate::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZHM3ItemTemplate::m_Id;
        mask = ZHM3ItemTemplate::m_Mask;
    }

    // vtbl slot 15 (RTTI)  PC 0x64D420 -> &ZHM3ItemTemplate::m_OldClassInfo
    Glacier::ZGEOMCLASSINFO* ZHM3ItemTemplate::GetOldClassInfo() const
    {
        return ZHM3ItemTemplate::m_OldClassInfo;
    }

    // PC 0x510010
    EHM3ItemType ZHM3ItemTemplate::GetHM3ItemType()
    {
        return m_eHM3ItemType;
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        // The PC chain is laid out tail-first in memory; the head (m_szHM3NormalHoldAnim) is the
        // FirstProperty passed to DECLARE_GEOM_CLASS_IMPL. Game offset -> class offset = +4
        // (ZHM3ItemTemplate::Info.First is 0x80F658).
        static Glacier::RTP::ZDataProperty<bool> Edible{
            .m_Node = {.m_Next = nullptr, .m_Name = "m_bEdible", .m_Filter = 1},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_bool,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemTemplate, m_bEdible)};

        static Glacier::RTP::ZDataProperty<bool> Drinkable{
            .m_Node = {.m_Next = Edible, .m_Name = "m_bDrinkable", .m_Filter = 1},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_bool,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemTemplate, m_bDrinkable)};

        static Glacier::RTP::ZDataProperty<uint8_t> NumBites{
            .m_Node = {.m_Next = Drinkable, .m_Name = "m_iNumBites", .m_Filter = 1},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_uchar,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemTemplate, m_iNumBites)};

        static Glacier::RTP::ZDataProperty<int> ActorHoldAnimIdx{
            .m_Node = {.m_Next = NumBites, .m_Name = "m_nActorHoldAnimIdx", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_int,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemTemplate, m_nActorHoldAnimIdx)};

        static Glacier::RTP::ZDataProperty<int> RunHoldAnimIdx{
            .m_Node = {.m_Next = ActorHoldAnimIdx, .m_Name = "m_nHM3RunHoldAnimIdx", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_int,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemTemplate, m_nHM3RunHoldAnimIdx)};

        static Glacier::RTP::ZDataProperty<int> NormalHoldAnimIdx{
            .m_Node = {.m_Next = RunHoldAnimIdx, .m_Name = "m_nHM3NormalHoldAnimIdx", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_int,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemTemplate, m_nHM3NormalHoldAnimIdx)};

        static Glacier::RTP::ZDataProperty<Glacier::ZRTString> ActorHoldAnim{
            .m_Node = {.m_Next = NormalHoldAnimIdx, .m_Name = "m_szActorHoldAnim", .m_Filter = 1},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_ZRTString,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemTemplate, m_szActorHoldAnim)};

        static Glacier::RTP::ZDataProperty<Glacier::ZRTString> RunHoldAnim{
            .m_Node = {.m_Next = ActorHoldAnim, .m_Name = "m_szHM3RunHoldAnim", .m_Filter = 1},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_ZRTString,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemTemplate, m_szHM3RunHoldAnim)};

        static Glacier::RTP::ZDataProperty<Glacier::ZRTString> NormalHoldAnim{
            .m_Node = {.m_Next = RunHoldAnim, .m_Name = "m_szHM3NormalHoldAnim", .m_Filter = 1},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_ZRTString,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemTemplate, m_szHM3NormalHoldAnim)};
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZHM3ItemTemplate,        // ClassName
        Glacier::ZItemTemplate,  // BaseClass
        0x009B15B8,              // OldClassInfoAddr
        "ZHM3ItemTemplate",      // FactoryName
        0x0,                     // FactoryNameAddr (documentation only; not bound by the macro)
        cProperties::NormalHoldAnim, // FirstProperty
        0x0080F66C,              // PropertiesAddr (ZHM3ItemTemplate::Info)
        0x009B14F0,              // IdAddr (ZHM3ItemTemplate::m_Id)
        0x009B14F4               // MaskAddr (ZHM3ItemTemplate::m_Mask)
    );
#   pragma endregion
}
