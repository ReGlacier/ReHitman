#include <Glacier/Geom/ZGateLightOmni.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/RTP/VirtualTables.h>


namespace Glacier
{
    ZGateLightOmni::ZGateLightOmni(const char* psName, ZBaseGeom* pBaseGeom)
        : ZOMNILIGHT(psName, pBaseGeom)
        , m_lType(EType::CLONES)
        , m_RoomsFilter(1, 0)
        , m_lFilterType(EFilterType::EXCLUDE)
        , m_Slaves(4, 0)
        , m_Gates(4, 0)
    {
    }

    ZGateLightOmni::~ZGateLightOmni()
    {
        for (const uint32_t lSlaveRef : m_Slaves)
        {
            if (auto* pSlave = ZGEOM::RefToPtr(lSlaveRef))
            {
                pSlave->Delete();
            }
        }
    }

    const RTP::ZPropertyInfo& ZGateLightOmni::GetProperties() const
    {
        return ZGateLightOmni::Info;
    }

    uint32_t ZGateLightOmni::GetObjectId() const
    {
        return ZGateLightOmni::m_Id;
    }

    void ZGateLightOmni::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZGateLightOmni::m_Id;
        mask = ZGateLightOmni::m_Mask;
    }

    ZGEOMCLASSINFO* ZGateLightOmni::GetOldClassInfo() const
    {
        return ZGateLightOmni::m_OldClassInfo;
    }

    void ZGateLightOmni::Activate(bool bActive)
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

    void ZGateLightOmni::SetMat(const Glacier::ZMat3x3& mMat)
    {
        ZGEOM::SetMat(mMat);
    }

    void ZGateLightOmni::SetPos(const Glacier::ZVector3& vPos)
    {
        ZGEOM::SetPos(vPos);
    }

    void ZGateLightOmni::SetMatPos(const Glacier::ZMat3x3& mat, const Glacier::Vector3& pos)
    {
        ZGEOM::SetMatPos(mat, pos);
    }

    void ZGateLightOmni::Display(bool OnOff)
    {
        for (const uint32_t lSlaveRef : m_Slaves)
        {
            if (auto* pSlave = ZGEOM::RefToPtr(lSlaveRef))
            {
                pSlave->Display(OnOff);
            }
        }

        ZGEOM::Display(OnOff);
    }

    void ZGateLightOmni::ClassInit2()
    {
        ZASSERT(m_rMasterLight == 0);

        const uint32_t lRef = GetRef();

        for (const uint32_t lSlaveRef : m_Slaves)
        {
            auto* pSlave = ZGEOM::RefToPtr(lSlaveRef);
            if (pSlave && pSlave->BaseGeom() != BaseGeom())
            {
                static_cast<ZLIGHT*>(pSlave)->m_rMasterLight = lRef;
            }
        }
    }

    void ZGateLightOmni::CopyData(const ZGEOM* Source)
    {
        m_lType = EType::CLONES;
        m_lFilterType = EFilterType::EXCLUDE;

        m_RoomsFilter.Clear();
        m_Slaves.Clear();
        m_Gates.Clear();

        if (const auto* pSource = geom_cast<ZGateLightOmni>(Source))
        {
            m_lType = pSource->m_lType;
            m_lFilterType = pSource->m_lFilterType;

            for (int i = 0; i < pSource->m_RoomsFilter.Count(); ++i)
            {
                m_RoomsFilter.Add(pSource->m_RoomsFilter.GetRefNr(i));
            }

            for (int i = 0; i < pSource->m_Slaves.Count(); ++i)
            {
                m_Slaves.Add(pSource->m_Slaves.GetRefNr(i));
            }

            for (int i = 0; i < pSource->m_Gates.Count(); ++i)
            {
                m_Gates.Add(pSource->m_Gates.GetRefNr(i));
            }
        }

        ZLIGHT::CopyData(Source);
    }

    void ZGateLightOmni::Enable()
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

    void ZGateLightOmni::Disable()
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

    void ZGateLightOmni::SetMultiplier(float fValue)
    {
        for (const uint32_t lSlaveRef : m_Slaves)
        {
            if (auto* pSlave = ZGEOM::RefToPtr(lSlaveRef))
            {
                static_cast<ZLIGHT*>(pSlave)->SetMultiplier(fValue);
            }
        }

        ZOMNILIGHT::SetMultiplier(fValue);
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

        static RTP::ZDataProperty<REFTAB> NamespaceItem_4011
        {
            .m_Node = {
                .m_Next = nullptr,
                .m_Name = "m_Gates",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_DP__3,
            .m_Offset = CLASS_PROPERTY(ZGateLightOmni, m_Gates)
        };

        static RTP::ZDataProperty<REFTAB> NamespaceItem_4010
        {
            .m_Node = {
                .m_Next = NamespaceItem_4011,
                .m_Name = "m_Slaves",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_DP__3,
            .m_Offset = CLASS_PROPERTY(ZGateLightOmni, m_Slaves)
        };

        static RTP::ZEnumProperty NamespaceItem_4009
        {
            .m_Node = {
                .m_Next = NamespaceItem_4010,
                .m_Name = "m_lFilterType",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_EP,
            .m_Offset = CLASS_PROPERTY(ZGateLightOmni, m_lFilterType),
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
            .m_Offset = CLASS_PROPERTY(ZGateLightOmni, m_RoomsFilter)
        };

        static RTP::ZEnumProperty NamespaceItem_4007
        {
            .m_Node = {
                .m_Next = NamespaceItem_4008,
                .m_Name = "m_lType",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_EP,
            .m_Offset = CLASS_PROPERTY(ZGateLightOmni, m_lType),
            .m_Info = &NamespaceItem_4005
        };
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZGateLightOmni,
        ZOMNILIGHT,
        0x00972CF8,
        "ZGateLightOmni",
        0x0076BE6C,
        cProperties::NamespaceItem_4007,
        0x008144C0,
        0x00972B08,
        0x00972B0C
    );
#   pragma endregion
}
