#include <Glacier/Geom/ZBackdrop.h>
#include <Glacier/Geom/ZGeomBuffer.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/ZSTL/REFTAB32.h>


namespace Glacier
{
    ZBackdrop::ZBackdrop(const char* psName, ZBaseGeom* pBaseGeom)
        : ZROOM(psName, pBaseGeom)
        , m_Groups()
    {
    }

    ZBackdrop::~ZBackdrop() = default;

    const RTP::ZPropertyInfo& ZBackdrop::GetProperties() const
    {
        return ZBackdrop::Info;
    }

    uint32_t ZBackdrop::GetObjectId() const
    {
        return ZBackdrop::m_Id;
    }

    void ZBackdrop::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZBackdrop::m_Id;
        mask = ZBackdrop::m_Mask;
    }

    ZGEOMCLASSINFO* ZBackdrop::GetOldClassInfo() const
    {
        return ZBackdrop::m_OldClassInfo;
    }

    void ZBackdrop::ClassInit()
    {
        m_lGroupCon |= ZCINACTIVE;

        ZMat3x3 mRoot;
        ZVector3 vRoot;
        GetRootTM(mRoot, vRoot);

        if (mRoot.data[6] == 1.0f && mRoot.data[4] == 1.0f && mRoot.data[2] == 1.0f && vzero(vRoot))
        {
            for (uint32_t lEntryNr = 0; lEntryNr < m_Groups.Count(); ++lEntryNr)
            {
                auto* pEntry = *m_Groups.Get(lEntryNr);

                for (const auto* pBaseGeom = pEntry->BaseGeom(); pBaseGeom; pEntry->RecurGetNextGroup(&pBaseGeom))
                {
                    auto* pParentGroup = pBaseGeom->ParentGroup();
                    if (pBaseGeom == pParentGroup->BaseGeom())
                        break;

                    auto* pRoom = pBaseGeom->GetGeom();
                    if (pBaseGeom->IsDerivedFrom<ZROOM>())
                    {
                        SetAttachedRoom(static_cast<ZROOM*>(pRoom));
                    }
                }
            }

            if (m_Groups.IsEmpty())
            {
                ZWARN2("GeomList in ZBackdrop is empty - backdrop will never be drawn!");
            }
        }
    }

    void ZBackdrop::SetDynamicContainer(const bool& dynamic_container)
    {
        // DronCode: weird shit
        MakeDynamicContainer(false);
    }

    void ZBackdrop::GetGeomList(REFTAB32& aGeomList)
    {
        ZASSERT(false); // Not implemented by IOI or removed from prod build (weird, because in PS2 0.5 debug build it's still 'assert')
    }

    void ZBackdrop::SetGeomList(const REFTAB32& aGeomList)
    {
        for (auto rGeom : aGeomList.As<ZREF>())
        {
            auto* pGeom = ZGEOM::RefToPtr(rGeom);
            if (pGeom && pGeom->IsDerivedFrom<ZGROUP>())
            {
                auto* pGroup = reinterpret_cast<ZGROUP*>(pGeom);
                m_Groups.Add(&pGroup);
            }
        }
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        static RTP::ZVirtualProperty<REFTAB32> NamespaceItem_2090
        {
            .m_Node = {
                .m_Next = nullptr,
                .m_Name = "GeomList",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_VP__4,
            .m_Get = &ZBackdrop::GetGeomList,
            .m_Set = &ZBackdrop::SetGeomList
        };
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZBackdrop,
        ZROOM,
        0x0097B598,
        "ZBackdrop",
        0x0076E9AC,
        cProperties::NamespaceItem_2090,
        0x0080E368,
        0x0097B4DC,
        0x0097B4E0
    );
#   pragma endregion
}
