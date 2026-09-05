#include <Glacier/Geom/ZLIST.h>
#include <Glacier/RTP/VirtualTables.h>
#include <cassert>
#include <cstring>


namespace Glacier
{
    // ZLIST

    ZLIST::ZLIST(const char* psName, ZBaseGeom* pBaseGeom)
        : ZGEOM(psName, pBaseGeom)
    {
        m_pZList = nullptr;

        if (!std::strcmp(BaseGeom()->Name(), "TrackLinkObjects"))
        {
            m_TrackLinkObjectsInstance = this;
        }
    }

    ZLIST::~ZLIST()
    {
        if (m_pZList)
        {
            ZUniMemory::Delete(m_pZList);
            m_pZList = nullptr;
        }

        if (!std::strcmp(BaseGeom()->Name(), "TrackLinkObjects"))
        {
            assert(m_TrackLinkObjectsInstance == this);
            m_TrackLinkObjectsInstance = nullptr;
        }
    }

    bool ZLIST::PostLoad(ISerializerStream& stream)
    {
        if (!std::strcmp(BaseGeom()->Name(), "TrackLinkObjects"))
        {
            m_TrackLinkObjectsInstance = this;
        }

        return true;
    }

    // RTP::cBase
    const RTP::ZPropertyInfo& ZLIST::GetProperties() const
    {
        return ZLIST::Info;
    }

    uint32_t ZLIST::GetObjectId() const
    {
        return ZLIST::m_Id;
    }

    void ZLIST::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZLIST::m_Id;
        mask = ZLIST::m_Mask;
    }

    ZGEOMCLASSINFO* ZLIST::GetOldClassInfo() const
    {
        return ZLIST::m_OldClassInfo;
    }

    void ZLIST::CalcCenSize()
    {
        SetCen(ZVector3(0.0f, 0.0f, 0.0f));
        SetSize(ZVector3(0.0f, 0.0f, 0.0f));
        SetRadius(0.0f);
    }

    void ZLIST::SendCommandRecursive(ZMSGID Msg, void* pData, ZGEOM* pTarget)
    {
        ZGEOM::SendCommandRecursive(Msg, pData, pTarget);

        if (m_pZList)
        {
            for (const uint32_t lRef : *m_pZList)
            {
                if (auto* pGeom = ZGEOM::RefToPtr(lRef))
                {
                    pGeom->SendCommandRecursive(Msg, pData, pTarget);
                }
            }
        }
    }

    bool ZLIST::CheckPointInside(ZVector3& pPoint, float fDotDist)
    {
        if (m_pZList)
        {
            for (const uint32_t lRef : *m_pZList)
            {
                if (auto* pGeom = ZGEOM::RefToPtr(lRef))
                {
                    if (pGeom->CheckPointInside(pPoint, fDotDist))
                    {
                        return true;
                    }
                }
            }
        }

        return false;
    }

    void ZLIST::SendCommandToList(Glacier::ZMSGID command, Glacier::ZDATA data)
    {
        if (m_pZList)
        {
            for (const uint32_t lRef : *m_pZList)
            {
                if (auto* pGeom = ZGEOM::RefToPtr(lRef))
                {
                    pGeom->SendCommand(command, data, nullptr);
                }
            }
        }
    }

    void ZLIST::AddGeom(ZREF rGeom)
    {
        if (!m_pZList)
        {
            m_pZList = ZUniMemory::New<REFTAB>(32, 0);
        }

        if (!m_pZList->Exists(rGeom))
        {
            m_pZList->Add(rGeom);
        }
    }

    void ZLIST::AddGeom(ZGEOM* pGeom)
    {
        AddGeom(pGeom->GetRef());
    }

    void ZLIST::RemoveGeomById(ZREF rGeom)
    {
        if (m_pZList)
        {
            m_pZList->Remove(rGeom);
        }
    }

    void ZLIST::RemoveGeomByPtr(ZGEOM* pGeom)
    {
        RemoveGeomById(pGeom->GetRef());
    }

    void ZLIST::AddMeshNrs(MeshNrs*)
    {
        // Do nothing
    }

    int ZLIST::Script_GetRefNr(int lNr)
    {
        return m_pZList->GetRefNr(lNr);
    }

    int ZLIST::Script_GetCount()
    {
        return m_pZList->Count();
    }

    void ZLIST::GetGeomList(REFTAB32& rRefTab)
    {
        if (m_pZList)
        {
            for (const uint32_t lRef : *m_pZList)
            {
                rRefTab.Add(lRef);
            }
        }
    }

    void ZLIST::SetGeomList(const REFTAB32& rRefTab)
    {
        if (m_pZList)
        {
            ZUniMemory::Delete(m_pZList);
            m_pZList = nullptr;
        }

        m_pZList = ZUniMemory::New<REFTAB>(32, 0);

        const int lCount = rRefTab.Count();
        for (int i = 0; i < lCount; ++i)
        {
            const uint32_t lRef = rRefTab.GetRefNr(i);
            if (ZGEOM::RefToPtr(lRef))
            {
                m_pZList->Add(lRef);
            }
        }
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        static RTP::ZVirtualProperty<REFTAB32> NamespaceItem_1337
        {
            .m_Node = {
                .m_Next = nullptr,
                .m_Name = "GeomList",
                .m_Filter = 3
            },
            .m_VirtualTable = VirtualTable_VP__4,
            .m_Get = &ZLIST::GetGeomList,
            .m_Set = &ZLIST::SetGeomList

        };
    }
    DECLARE_GEOM_CLASS_IMPL(
        ZLIST,
        ZGEOM,
        0x00972DF8,
        "ZLIST",
        0x0076C0D4,
        cProperties::NamespaceItem_1337,
        0x008068FC,
        0x00972DA0,
        0x00972DA4
    );
#   pragma endregion

    // Globals
    STATIC_CLASS_VAR_IMPL(ZLIST, ZLIST*, m_TrackLinkObjectsInstance, 0x00972DA8, nullptr);
}
