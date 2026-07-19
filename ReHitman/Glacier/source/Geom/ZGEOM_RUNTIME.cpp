#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Runtime/ZEnum.h>
#include <Glacier/RTP/Base.h>


namespace Glacier
{
     // TODO: Finish VirtualTable reconstruction!
    namespace cProperties
    {
        static ZEnumEntry NamespaceItem_3409_BOUNDING_Static
        { 
            .m_Prev = nullptr, 
            .m_Value = static_cast<int>(EBoundingBox::BOUNDING_Static), 
            .m_Name = "BOUNDING_Dynamic" 
        };

        static ZEnumEntry NamespaceItem_3409_BOUNDING_Dynamic 
        {
            .m_Prev = &NamespaceItem_3409_BOUNDING_Static,
            .m_Value = static_cast<int>(EBoundingBox::BOUNDING_Dynamic),
            .m_Name = "BOUNDING_Dynamic"
        };

        static ZEnumEntry NamespaceItem_3409_BOUNDING_DynamicAutoAssign 
        {
            .m_Prev = &NamespaceItem_3409_BOUNDING_Dynamic,
            .m_Value = static_cast<int>(EBoundingBox::BOUNDING_DynamicAutoAssign),
            .m_Name = "BOUNDING_DynamicAutoAssign"
        };

        static ZEnumInfo NamespaceItem_3409 
        {
            .m_Last = &NamespaceItem_3409_BOUNDING_DynamicAutoAssign,
            .m_Name = "EBoundingBox",
            .m_Size = 4 // Count of elements
        };

        struct RTP::ZVirtualProperty<float[9]> NamespaceItem_3414 {
            .m_Node {
                .m_Next = nullptr, // TODO: FINISH ME
                .m_Name = "Matrix",
                .m_Filter = 1
            },
            .m_VirtualTable = nullptr, // TODO: Fixme
            .m_Get = { .__pfn = static_cast<void(ZSerializableBase::*)(float(&)[9])>(&ZGEOM::GetMatrix) },
            .m_Set = { .__pfn = static_cast<void(ZSerializableBase::*)(const float(&)[9])>(&ZGEOM::SetMatrix) }
        };

        static RTP::ZVirtualEnumProperty<EBoundingBox> NamespaceItem_3413 
        {
            .m_Node {
                .m_Next = &NamespaceItem_3414.m_Node,
                .m_Name = "BoundingBox",
                .m_Filter = 1
            },
            .m_VirtualTable = nullptr, // TODO: FIXME
            .m_Get = { .__pfn = static_cast<void(ZSerializableBase::*)(EBoundingBox&)>(&ZGEOM::GetBoundingBox) },
            .m_Set = { .__pfn = static_cast<void(ZSerializableBase::*)(const EBoundingBox&)>(&ZGEOM::SetBoundingBox) },
            .m_Info = &NamespaceItem_3409
        };
    }


    // Entry
    STATIC_CLASS_VAR_IMPL(ZGEOM, RTP::ZPropertyInfo, Info, 0x00806848, (RTP::ZPropertyInfo {
        .First = &cProperties::NamespaceItem_3413.m_Node,
        .Super = &RTP::cBase::Info,
        .Name = ZGEOM::FactoryName
    }));
}