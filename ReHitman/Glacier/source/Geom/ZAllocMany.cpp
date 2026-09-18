#include <Glacier/Geom/ZAllocMany.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <cstring>


namespace Glacier
{
    ZAllocMany::ZAllocMany(const char* psName, ZBaseGeom* pBaseGeom)
        : ZGROUP(psName, pBaseGeom)
    {
    }

    ZAllocMany::~ZAllocMany()
    {
        if (m_pFreeObjects)
        {
            ZUniMemory::Free(m_pFreeObjects);
        }
    }

    void ZAllocMany::PostSave(ISerializerStream& stream)
    {
        if (!m_bClone && stream.TestStreamFilter(1u << ISerializerStream::CONTENT_SavedGame))
        {
            for (uint16_t i = 0; i < m_iNumObjects; ++i)
            {
                uint32_t ref = m_pFreeObjects[i]->GetRef();
                stream.Exchange("FreeRef", ref);
            }
        }
    }

    bool ZAllocMany::PostLoad(ISerializerStream& stream)
    {
        if (m_bClone || !stream.TestStreamFilter(1u << ISerializerStream::CONTENT_SavedGame))
            return true;

        ZASSERT(m_pFreeObjects);
        for (uint16_t i = 0; i < m_iNumObjects; ++i)
        {
            uint32_t ref = 0;
            stream.Exchange("FreeRef", ref);
            m_pFreeObjects[i] = static_cast<ZAllocMany*>(ZGEOM::RefToPtr(ref));
        }
        return true;
    }

    const RTP::ZPropertyInfo& ZAllocMany::GetProperties() const
    {
        return ZAllocMany::Info;
    }

    uint32_t ZAllocMany::GetObjectId() const
    {
        return ZAllocMany::m_Id;
    }

    void ZAllocMany::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZAllocMany::m_Id;
        mask = ZAllocMany::m_Mask;
    }

    ZGEOMCLASSINFO* ZAllocMany::GetOldClassInfo() const
    {
        return ZAllocMany::m_OldClassInfo;
    }

    void ZAllocMany::Activate(bool bActive)
    {
        ZGEOM::Activate(bActive);

        if (m_pFreeObjects)
        {
            for (uint16_t i = 0; i < m_iNumObjects; ++i)
            {
                if (bActive)
                    m_pFreeObjects[i]->MakeActive();
                else
                    m_pFreeObjects[i]->MakeInactive();
            }
        }
    }

    void ZAllocMany::ClassInit2()
    {
        if (!stricmp(GetOldClassInfo()->ClassInfoName(), "ZCigs"))
        {
            for (ZBaseGeom* pBase = BaseGeom(); pBase; RecurGetNext(&pBase))
            {
                auto* pGeom = pBase->GetGeom();

                // TODO: Finish this place after ZParticleEmitter will be reversed
                // PC decompile:
                //   if ((pGeom->GetObjectId() & ZParticleEmitter::m_Mask) == ZParticleEmitter::m_Id)
                //       *(uint8_t*)((char*)pGeom + 0x74) &= ~0x8;
                //   else if (stricmp(pGeom->GetOldClassInfo()->ClassInfoName(), "ZCigs") != 0)
                //       pBase->Hide(true);
                if (stricmp(pGeom->GetOldClassInfo()->ClassInfoName(), "ZCigs") != 0)
                {
                    pBase->Hide(true);
                }
            }
        }

        if (m_bClone)
        {
            if (stricmp(GetOldClassInfo()->ClassInfoName(), "ZCigs") != 0)
            {
                MakeInactive();
            }

            for (ZBaseGeom* pBase = BaseGeom(); pBase; RecurGetNext(&pBase))
            {
                pBase->MakeInactive();
            }
        }
        else
        {
            m_pFreeObjects = reinterpret_cast<ZAllocMany**>(ZUniMemory::Allocate(sizeof(ZAllocMany*) * m_iNumObjects));
        }
    }

    void ZAllocMany::PostClassInit()
    {
        if (m_iNumObjects && !m_bClone && m_bInitClones)
        {
            m_iNumFree = m_iNumObjects;
            if (auto* pCloneGroup = static_cast<ZGROUP*>(ZGEOM::RefToPtr(m_rCloneGroup)))
            {
                int i = 0;
                for (auto* pBase = pCloneGroup->m_pGroupFirst; pBase; pBase = pBase->Next())
                {
                    auto* pGeom = pBase->GetGeom();
                    ZASSERT(pGeom);
                    static_cast<ZAllocMany*>(pGeom)->m_pOriginal = this;
                    m_pFreeObjects[i++] = static_cast<ZAllocMany*>(pGeom);
                }
                ZASSERT(i == m_iNumObjects);
            }
            m_bInitClones = 0;
        }
    }

    ZGEOM* ZAllocMany::Duplicate(ZGROUP* DestGroup, const char* DupName, bool Recursive)
    {
        ZASSERT(!m_bClone);
        ZASSERT((m_Mask & DestGroup->GetObjectId()) != m_Id);

        uint16_t v5 = m_iFreeIndex;
        ZAllocMany* pObject = m_pFreeObjects[v5++];
        --m_iNumFree;
        m_iFreeIndex = v5;
        if (v5 == m_iNumObjects)
            m_iFreeIndex = 0;

        if (pObject)
        {
            pObject->MakeActiveRecursive();
            pObject->SendCommandRecursive(m_msgActivate, nullptr, nullptr);
            DestGroup->AttachGeom(pObject, true);
        }
        return pObject;
    }

    ZGEOM* ZAllocMany::DuplicateInit(ZGROUP* DestGroup, const ZMat3x3* pMat, const ZVector3* pPos, char const* DupName, bool Recursive)
    {
        ZASSERT(!m_bClone);
        ZASSERT((m_Mask & DestGroup->GetObjectId()) != m_Id);

        uint16_t v7 = m_iFreeIndex;
        ZAllocMany* pObject = m_pFreeObjects[v7++];
        --m_iNumFree;
        m_iFreeIndex = v7;
        if (v7 == m_iNumObjects)
            m_iFreeIndex = 0;

        if (pObject)
        {
            DestGroup->AttachGeom(pObject, true);
            pObject->SetMatPos(*pMat, *pPos);
            pObject->MakeActiveRecursive();
            pObject->SendCommandRecursive(m_msgActivate, nullptr, nullptr);
        }
        return pObject;
    }

    void ZAllocMany::CopyData(const ZGEOM* Source)
    {
        ZGROUP::CopyData(Source);

        if (auto* pSource = geom_cast<ZAllocMany>(Source))
        {
            m_iNumObjects = pSource->m_iNumObjects;
        }
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        static RTP::ZDataProperty<bool> NamespaceItem_3133
        {
            .m_Node = {
                .m_Next = nullptr,
                .m_Name = "m_bInitClones",
                .m_Filter = 2
            },
            .m_VirtualTable = VirtualTable_DP__1,
            .m_Offset = CLASS_PROPERTY(ZAllocMany, m_bInitClones)
        };

        static RTP::ZDataProperty<ZGEOMREF> NamespaceItem_3132
        {
            .m_Node = {
                .m_Next = NamespaceItem_3133,
                .m_Name = "m_rCloneGroup",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_DP__5,
            .m_Offset = reinterpret_cast<ZGEOMREF*>(CLASS_PROPERTY(ZAllocMany, m_rCloneGroup))
        };

        static RTP::ZDataProperty<bool> NamespaceItem_3131
        {
            .m_Node = {
                .m_Next = NamespaceItem_3132,
                .m_Name = "m_bClone",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_DP__1,
            .m_Offset = CLASS_PROPERTY(ZAllocMany, m_bClone)
        };

        static RTP::ZDataProperty<ZMsg> NamespaceItem_3130
        {
            .m_Node = {
                .m_Next = NamespaceItem_3131,
                .m_Name = "m_msgActivate",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_DP__15,
            .m_Offset = reinterpret_cast<ZMsg*>(CLASS_PROPERTY(ZAllocMany, m_msgActivate))
        };

        static RTP::ZDataProperty<ZAllocMany*> NamespaceItem_3129
        {
            .m_Node = {
                .m_Next = NamespaceItem_3130,
                .m_Name = "m_pOriginal",
                .m_Filter = 2
            },
            .m_VirtualTable = VirtualTable_DP__178,
            .m_Offset = CLASS_PROPERTY(ZAllocMany, m_pOriginal)
        };

        static RTP::ZDataProperty<bool> NamespaceItem_3128
        {
            .m_Node = {
                .m_Next = NamespaceItem_3129,
                .m_Name = "m_bNeedClean",
                .m_Filter = 2
            },
            .m_VirtualTable = VirtualTable_DP__1,
            .m_Offset = CLASS_PROPERTY(ZAllocMany, m_bNeedClean)
        };

        static RTP::ZDataProperty<short> NamespaceItem_3127
        {
            .m_Node = {
                .m_Next = NamespaceItem_3128,
                .m_Name = "m_iFreeIndex",
                .m_Filter = 2
            },
            .m_VirtualTable = VirtualTable_DP__13,
            .m_Offset = CLASS_PROPERTY(ZAllocMany, m_iFreeIndex)
        };

        static RTP::ZDataProperty<short> NamespaceItem_3126
        {
            .m_Node = {
                .m_Next = NamespaceItem_3127,
                .m_Name = "m_iNumFree",
                .m_Filter = 2
            },
            .m_VirtualTable = VirtualTable_DP__13,
            .m_Offset = CLASS_PROPERTY(ZAllocMany, m_iNumFree)
        };

        static RTP::ZDataProperty<short> NamespaceItem_3125
        {
            .m_Node = {
                .m_Next = NamespaceItem_3126,
                .m_Name = "m_iNumObjects",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_DP__13,
            .m_Offset = CLASS_PROPERTY(ZAllocMany, m_iNumObjects)
        };
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZAllocMany,
        ZGROUP,
        0x0097B9B8,
        "ZAllocMany",
        0x007701FC,
        cProperties::NamespaceItem_3125,
        0x0080F1FC,
        0x0097B968,
        0x0097B96C
    );
#   pragma endregion
}
