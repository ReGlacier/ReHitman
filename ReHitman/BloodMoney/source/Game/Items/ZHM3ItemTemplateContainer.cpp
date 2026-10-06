#include <BloodMoney/Game/Items/ZHM3ItemTemplateContainer.h>

#include <Glacier/RTP/VirtualTables.h>

namespace Hitman
{
    // PC 0x6501B0. Chains to the ZItemTemplateContainer base ctor.
    ZHM3ItemTemplateContainer::ZHM3ItemTemplateContainer(const char* psName, Glacier::ZBaseGeom* pBaseGeom)
        : Glacier::ZItemTemplateContainer(psName, pBaseGeom)
    {
    }

    // vtbl slot 12 (RTTI)  PC 0x64D690
    const Glacier::RTP::ZPropertyInfo& ZHM3ItemTemplateContainer::GetProperties() const
    {
        return ZHM3ItemTemplateContainer::Info;
    }

    // vtbl slot 13 (RTTI)  PC 0x64D6D0
    uint32_t ZHM3ItemTemplateContainer::GetObjectId() const
    {
        return ZHM3ItemTemplateContainer::m_Id;
    }

    // vtbl slot 14 (RTTI)  PC 0x64D6E0
    void ZHM3ItemTemplateContainer::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZHM3ItemTemplateContainer::m_Id;
        mask = ZHM3ItemTemplateContainer::m_Mask;
    }

    // vtbl slot 15 (RTTI)  PC 0x64D6A0 -> &ZHM3ItemTemplateContainer::m_OldClassInfo
    Glacier::ZGEOMCLASSINFO* ZHM3ItemTemplateContainer::GetOldClassInfo() const
    {
        return ZHM3ItemTemplateContainer::m_OldClassInfo;
    }

    // PC 0x64D6C0
    EHM3ItemType ZHM3ItemTemplateContainer::GetHM3ItemType()
    {
        return m_eHM3ItemType;
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        // The PC chain is laid out tail-first; the head (m_szHM3NormalHoldAnim) is the FirstProperty
        // passed to DECLARE_GEOM_CLASS_IMPL. Game offset -> class offset = +4
        // (ZHM3ItemTemplateContainer::Info.First is 0x80F74C). The PC m_Name pointers are null, so
        // the names come from the PS2 records (0x94C1F8..); m_eHM3ItemType (class offset 0x98) is
        // not serialized.
        static Glacier::RTP::ZDataProperty<int> ActorHoldAnimIdx{
            .m_Node = {.m_Next = nullptr, .m_Name = "m_nActorHoldAnimIdx", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_int,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemTemplateContainer, m_nActorHoldAnimIdx)};

        static Glacier::RTP::ZDataProperty<int> RunHoldAnimIdx{
            .m_Node = {.m_Next = ActorHoldAnimIdx, .m_Name = "m_nHM3RunHoldAnimIdx", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_int,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemTemplateContainer, m_nHM3RunHoldAnimIdx)};

        static Glacier::RTP::ZDataProperty<int> NormalHoldAnimIdx{
            .m_Node = {.m_Next = RunHoldAnimIdx, .m_Name = "m_nHM3NormalHoldAnimIdx", .m_Filter = 2},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_int,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemTemplateContainer, m_nHM3NormalHoldAnimIdx)};

        static Glacier::RTP::ZDataProperty<Glacier::ZRTString> ActorHoldAnim{
            .m_Node = {.m_Next = NormalHoldAnimIdx, .m_Name = "m_szActorHoldAnim", .m_Filter = 1},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_ZRTString,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemTemplateContainer, m_szActorHoldAnim)};

        static Glacier::RTP::ZDataProperty<Glacier::ZRTString> RunHoldAnim{
            .m_Node = {.m_Next = ActorHoldAnim, .m_Name = "m_szHM3RunHoldAnim", .m_Filter = 1},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_ZRTString,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemTemplateContainer, m_szHM3RunHoldAnim)};

        static Glacier::RTP::ZDataProperty<Glacier::ZRTString> NormalHoldAnim{
            .m_Node = {.m_Next = RunHoldAnim, .m_Name = "m_szHM3NormalHoldAnim", .m_Filter = 1},
            .m_VirtualTable = &Glacier::RTP::VirtualTables::Data_ZRTString,
            .m_Offset = CLASS_PROPERTY(ZHM3ItemTemplateContainer, m_szHM3NormalHoldAnim)};
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZHM3ItemTemplateContainer,        // ClassName
        Glacier::ZItemTemplateContainer,  // BaseClass
        0x009B1888,                       // OldClassInfoAddr
        "ZHM3ItemTemplateContainer",      // FactoryName
        0x0,                              // FactoryNameAddr (documentation only; not bound by the macro)
        cProperties::NormalHoldAnim,      // FirstProperty
        0x0080F760,                       // PropertiesAddr (ZHM3ItemTemplateContainer::Info)
        0x009B1538,                       // IdAddr (ZHM3ItemTemplateContainer::m_Id)
        0x009B153C                        // MaskAddr (ZHM3ItemTemplateContainer::m_Mask)
    );
#   pragma endregion
}
