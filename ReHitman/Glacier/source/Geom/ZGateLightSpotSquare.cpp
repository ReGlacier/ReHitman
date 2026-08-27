#include <Glacier/Geom/ZGateLightSpotSquare.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/Geom/ZGEOM.h>


namespace Glacier
{
    ZGateLightSpotSquare::ZGateLightSpotSquare(const char* psName, ZBaseGeom* pBaseGeom)
        : ZSPOTLIGHTSQUARE(psName, pBaseGeom)
        , m_lType(EType::CLONES)
        , m_RoomsFilter(1, 0)
        , m_lFilterType(EFilterType::EXCLUDE)
        , m_Slaves(4, 0)
    {
        // Do nothing
    }

    ZGateLightSpotSquare::~ZGateLightSpotSquare()
    {
        for (const uint32_t lSlaveRef : m_Slaves)
        {
            if (auto* pSlave = ZGEOM::RefToPtr(lSlaveRef))
            {
                pSlave->Delete();
            }
        }
    }

    const RTP::ZPropertyInfo& ZGateLightSpotSquare::GetProperties() const
    {
        return ZGateLightSpotSquare::Info;
    }

    uint32_t ZGateLightSpotSquare::GetObjectId() const
    {
        return ZGateLightSpotSquare::m_Id;
    }

    void ZGateLightSpotSquare::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZGateLightSpotSquare::m_Id;
        mask = ZGateLightSpotSquare::m_Mask;
    }

    ZGEOMCLASSINFO* ZGateLightSpotSquare::GetOldClassInfo() const
    {
        return ZGateLightSpotSquare::m_OldClassInfo;
    }

    void ZGateLightSpotSquare::Activate(bool bActive)
    {
        for (const uint32_t lSlaveRef : m_Slaves)
        {
            if (auto* pSlave = ZGEOM::RefToPtr(lSlaveRef))
            {
                pSlave->Activate(bActive);
            }
        }

        ZLIGHT::Activate(bActive);
    }

    void ZGateLightSpotSquare::CopyData(const ZGEOM* Source)
    {
        if (const auto* pSource = geom_cast<ZGateLightSpotSquare>(Source))
        {
            m_lType = pSource->m_lType;
            m_lFilterType = pSource->m_lFilterType;

            m_RoomsFilter.Clear();
            for (int i = 0; i < pSource->m_RoomsFilter.Count(); ++i)
            {
                m_RoomsFilter.Add(pSource->m_RoomsFilter.GetRefNr(i));
            }

            m_Slaves.Clear();
            for (int i = 0; i < pSource->m_Slaves.Count(); ++i)
            {
                m_Slaves.Add(pSource->m_Slaves.GetRefNr(i));
            }
        }

        ZLIGHT::CopyData(Source);
    }

    void ZGateLightSpotSquare::Enable()
    {
        for (const uint32_t lSlaveRef : m_Slaves)
        {
            if (auto* pSlave = ZGEOM::RefToPtr(lSlaveRef))
            {
                static_cast<ZLIGHT*>(pSlave)->Enable();
            }
        }

        BaseGeom()->SetControl(0, ZCINVISIBLE);
        SetLightCon(ZL_ATTRIBUTE_CHANGED, 9);
    }

    void ZGateLightSpotSquare::Disable()
    {
        for (const uint32_t lSlaveRef : m_Slaves)
        {
            if (auto* pSlave = ZGEOM::RefToPtr(lSlaveRef))
            {
                static_cast<ZLIGHT*>(pSlave)->Disable();
            }
        }

        BaseGeom()->SetControl(ZCINVISIBLE, 0);
        SetLightCon(9, 0);
    }

    void ZGateLightSpotSquare::SetMultiplier(float fValue)
    {
        for (const uint32_t lSlaveRef : m_Slaves)
        {
            if (auto* pSlave = ZGEOM::RefToPtr(lSlaveRef))
            {
                static_cast<ZLIGHT*>(pSlave)->SetMultiplier(fValue);
            }
        }

        ZSPOTLIGHTSQUARE::SetMultiplier(fValue);
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        static ZEnumEntry NamespaceItem_4005_CLONES
        {
            .m_Prev = nullptr,
            .m_Value = 0,
            .m_Name = "CLONES"
        };

        static ZEnumEntry NamespaceItem_4005_AIMATGATES
        {
            .m_Prev = &NamespaceItem_4005_CLONES,
            .m_Value = 1,
            .m_Name = "AIMATGATES"
        };

        static ZEnumInfo NamespaceItem_4005
        {
            .m_Last = &NamespaceItem_4005_AIMATGATES,
            .m_Name = "EType",
            .m_Size = 4
        };

        static ZEnumEntry NamespaceItem_4006_EXCLUDE
        {
            .m_Prev = nullptr,
            .m_Value = 0,
            .m_Name = "EXCLUDE"
        };

        static ZEnumEntry NamespaceItem_4006_INCLUDE
        {
            .m_Prev = &NamespaceItem_4006_EXCLUDE,
            .m_Value = 1,
            .m_Name = "INCLUDE"
        };

        static ZEnumInfo NamespaceItem_4006
        {
            .m_Last = &NamespaceItem_4006_INCLUDE,
            .m_Name = "EFilterType",
            .m_Size = 4
        };

        static RTP::ZDataProperty<REFTAB> NamespaceItem_4010
        {
            .m_Node = {
                .m_Next = nullptr,
                .m_Name = "m_Slaves",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_DP__3,
            .m_Offset = CLASS_PROPERTY(ZGateLightSpotSquare, m_Slaves)
        };

        static RTP::ZEnumProperty NamespaceItem_4009
        {
            .m_Node = {
                .m_Next = NamespaceItem_4010,
                .m_Name = "m_lFilterType",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_EP,
            .m_Offset = CLASS_PROPERTY(ZGateLightSpotSquare, m_lFilterType),
            .m_Info = &NamespaceItem_4006
        };

        static RTP::ZDataProperty<REFTAB> NamespaceItem_4008
        {
            .m_Node = {
                .m_Next = NamespaceItem_4009,
                .m_Name = "m_RoomsFilter",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_DP__3,
            .m_Offset = CLASS_PROPERTY(ZGateLightSpotSquare, m_RoomsFilter)
        };

        static RTP::ZEnumProperty NamespaceItem_4007
        {
            .m_Node = {
                .m_Next = NamespaceItem_4008,
                .m_Name = "m_lType",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_EP,
            .m_Offset = CLASS_PROPERTY(ZGateLightSpotSquare, m_lType),
            .m_Info = &NamespaceItem_4005
        };
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZGateLightSpotSquare,
        ZSPOTLIGHTSQUARE,
        0x00972D98,
        "ZGateLightSpotSquare",
        0x0076BE8C,
        cProperties::NamespaceItem_4007,
        0x00814600,
        0x00972B18,
        0x00972B1C
    );
#   pragma endregion
}
