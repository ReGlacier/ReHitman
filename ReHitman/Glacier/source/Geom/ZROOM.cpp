#include <Glacier/Geom/ZGeomBuffer.h>
#include <Glacier/Geom/ZROOM.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZLIGHT.h>
#include <Glacier/Audio/ZSoundDllBase.h>
#include <Glacier/ZSTL/StringUtils.h>
#include <Glacier/ZSTL/REFTAB32.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    ZROOM::ZROOM(const char* psName, ZBaseGeom* pBaseGeom)
        : ZTreeGroup(psName, pBaseGeom)
        , m_LightSwitches(2, 1)
        , m_FurnitureList(1, 0)
        , m_StaticShadowSampleData()
        , m_lCacheIndex(-1)
        , m_lSoundGraphId(-1)
        , m_pRoomCache(nullptr)
        , m_bIsOnDrawStack(false)
        , m_pTempRoom(nullptr)
        , m_pExits(nullptr)
        , m_lNrExits(0)
        , m_dwRoomRef(0)
        , m_pDynamicGeoms(nullptr)
        , m_lRoomControl(0)
        , m_pEnvironment(nullptr)
        , m_lRoomOccl(0)
        , m_lStaticGeomsCustomDrawList(0)
        , m_lStaticGeomsPrimDrawList(0)
        , m_lDynamicGeomsDrawList(0)
        , m_lFogColor(0)
        , m_eLocationState(ELocation::eUNDEFINED)
        , m_pActorsAwareOfBrokenLight(nullptr)
        , m_lMusicId(0)
        , m_iLastVisibleFrameCount(0)
        , m_iLightState(1)
        , m_eNoiseLevel(ENoiseLevel::eNormal)
    {
        m_lGroupCon &= ~0xFCFFFFFFu;
        m_rAttachedDrawBaseGeoms[0] = 0;
        m_rAttachedDrawBaseGeoms[1] = 0;
    }

    ZROOM::~ZROOM()
    {
        // Weird, it's duplicate of ZTreeGroup dtor
        while (m_pGroupFirst)
        {
            ZUniMemory::Delete(m_pGroupFirst);
        }

        FreeGeomsLists();
        if (m_pActorsAwareOfBrokenLight)
        {
            ZUniMemory::Delete(m_pActorsAwareOfBrokenLight);
            m_pActorsAwareOfBrokenLight = nullptr;
        }
    }

    bool ZROOM::PostLoad(ISerializerStream& stream)
    {
        m_lGroupCon &= ~0xFCFFFFFFu;

        return true;
    }

    void ZROOM::LoadSave(ISerializerStream& stream, bool bSaving)
    {
        ZGROUP::LoadSave(stream, bSaving);

        stream.Exchange("rAttached1", m_rAttachedDrawBaseGeoms[0]);
        stream.Exchange("rAttached2", m_rAttachedDrawBaseGeoms[1]);

        for (int i = 0; i < m_lNrExits; ++i)
        {
            stream.Exchange("ExitControl", m_pExits[i].m_lControl);
        }
    }

    const RTP::ZPropertyInfo& ZROOM::GetProperties() const
    {
        return ZROOM::Info;
    }

    uint32_t ZROOM::GetObjectId() const
    {
        return ZROOM::m_Id;
    }

    void ZROOM::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZROOM::m_Id;
        mask = ZROOM::m_Mask;
    }

    ZGEOMCLASSINFO* ZROOM::GetOldClassInfo() const
    {
        return ZROOM::m_OldClassInfo;
    }

    void ZROOM::ClassInit()
    {
        if (m_eLocationState != ELocation::eUNDEFINED)
            return;

        if (auto* pParentGroup = BaseGeom()->ParentGroup())
        {
            const char* pszParentName = pParentGroup->Name();
            if (striwcmp(pszParentName, "*inside*"))
            {
                if (!striwcmp(pszParentName, "*outside*"))
                {
                    m_eLocationState = ELocation::eOUTSIDE;
                }
            }
            else
            {
                m_eLocationState = ELocation::eINSIDE;
            }
        }
    }

    void ZROOM::PostClassInit2()
    {
        if (ZSoundDllBase* pSoundDll = g_pSysInterface->GetSoundDll(); pSoundDll && m_dwRoomRef)
        {
            m_lAudioFilter = static_cast<int32_t>(reinterpret_cast<uintptr_t>(
                pSoundDll->GetPackedObject(m_dwRoomRef)));
        }
    }

    void ZROOM::CopyData(const ZGEOM* Source)
    {
        if (const auto* pSource = geom_cast<ZROOM>(Source))
        {
            m_dwRoomRef = pSource->m_dwRoomRef;
            m_lSoundGraphId = pSource->m_lSoundGraphId;
            // DronCode: Next weird code; in PC there are unconditional assertion
            //           I skipped it due in PS2 it says ZASSERT(INEDITOR)
            //           Also, Mini Ninjas does not contains any assertions here
            m_lNrExits = pSource->m_lNrExits;
            m_pExits = pSource->m_pExits;
        }

        ZTreeGroup::CopyData(Source);
    }

    void ZROOM::SetStaticContainer(const bool& static_container)
    {
        MakeStaticContainer(static_container);
    }

    ZROOM* ZROOM::CalcBestRoom(ZREF rOldRoom, const ZMat3x3& mMat, const ZVector3& vPos, const ZVector3& s0)
    {
        ZERROR("ERROR: ZROOM::CalcBestRoom is not supported anymore!");
        return nullptr;
    }

    bool ZROOM::AddDynamicGeomToRoom(ZBaseGeom* pBaseGeom)
    {
        if (!m_pDynamicGeoms)
        {
            auto* p = reinterpret_cast<ZPoolAllocRefTab*>(ZROOM::s_DynamicGeomsAllocator.Alloc(sizeof(ZPoolAllocRefTab)));
            if (p)
            {
                new (p) ZPoolAllocRefTab(&ZROOM::s_DynamicGeomsAllocator);
            }
            else
            {
                ZASSERT(p);
            }

            m_pDynamicGeoms = p;
        }

        // Weird code, I know
        if (!m_pDynamicGeoms->Exists(reinterpret_cast<uint32_t>(pBaseGeom)))
        {
            m_pDynamicGeoms->Add(reinterpret_cast<uint32_t>(pBaseGeom));
        }

        return true;
    }

    void ZROOM::RemoveDynamicGeomFromRoom(ZBaseGeom* pBaseGeom)
    {
        const auto lBaseGeom = reinterpret_cast<uint32_t>(pBaseGeom); // I don't know why IOI didn't use ZREF here, it's weird shitcode

        if (m_pDynamicGeoms && m_pDynamicGeoms->Exists(lBaseGeom))
        {
            m_pDynamicGeoms->Remove(lBaseGeom);
            ZASSERT(!m_pDynamicGeoms->Exists(lBaseGeom));
        }
    }

    void ZROOM::SetRoomControl(uint32_t lAdd, uint32_t lRemove)
    {
        m_lRoomControl = lAdd | (m_lRoomControl & ~lRemove);
    }

    uint32_t ZROOM::RoomControl() const
    {
        return m_lRoomControl;
    }

    ZBaseGeom** ZROOM::GetDynamicLightsInRoom(ZBaseGeom** pDrawGeomsList, ZBaseGeom** pDrawGeomsListEnd)
    {
        if (!m_pDynamicGeoms)
        {
            return pDrawGeomsList;
        }

        for (auto* pBaseGeom : m_pDynamicGeoms->As<ZBaseGeom*>())
        {
            if (pDrawGeomsList >= pDrawGeomsListEnd)
            {
                return pDrawGeomsList;
            }

            if (pBaseGeom->IsDerivedFrom<ZLIGHT>())
            {
                *pDrawGeomsList++ = pBaseGeom;
            }
            else if (pBaseGeom->IsDerivedFrom<ZGROUP>())
            {
                auto* pGroupGeom = static_cast<ZGROUP*>(pBaseGeom->GetGeom());
                if ((pGroupGeom->GroupControl() & ZGROUP::ZGRPCF_GROUP_CONTAINS_LIGHT) != 0)
                {
                    for (ZBaseGeom* pCur = pGroupGeom->BaseGeom(); pCur; pGroupGeom->RecurGetNext(&pCur))
                    {
                        if (pCur->IsDerivedFrom<ZLIGHT>() && pDrawGeomsList < pDrawGeomsListEnd)
                        {
                            *pDrawGeomsList++ = pCur;
                        }
                    }
                }
            }
        }

        return pDrawGeomsList;
    }

    ZBaseGeom** ZROOM::GetStaticPrimDrawGeomsListsRecur(ZBaseGeom** pDrawGeomsList, ZBaseGeom** pDrawGeomsListEnd)
    {
        SGeomPairRecursion sRecur {};
        sRecur.InitPair(m_lStaticGeomsPrimDrawList);

        while (sRecur.DpInsertList && sRecur.m_cCur != sRecur.m_cCurEnd)
        {
            auto* pPayload = reinterpret_cast<uintptr_t*>(
                reinterpret_cast<char*>(sRecur.DpInsertList) + sizeof(SBaseGeomListHeader));

            const auto pFirstGeom = reinterpret_cast<ZBaseGeom*>(pPayload[sRecur.m_cCur] & ~uintptr_t(7));
            const auto pLastGeom = reinterpret_cast<ZBaseGeom*>(pPayload[sRecur.m_cCur + 1] & ~uintptr_t(7));
            sRecur.m_cCur += 2;

            if (pDrawGeomsList >= pDrawGeomsListEnd)
            {
                break;
            }

            *pDrawGeomsList++ = pFirstGeom;
            *pDrawGeomsList++ = pLastGeom;

            sRecur.NextPair();
        }

        return pDrawGeomsList;
    }

    ZBaseGeom** ZROOM::GetStaticCustomDrawGeomsListsRecur(ZBaseGeom** pDrawGeomsList, ZBaseGeom** pDrawGeomsListEnd)
    {
        SGeomPairRecursion sRecur {};
        sRecur.InitPair(m_lStaticGeomsCustomDrawList);

        while (sRecur.DpInsertList && sRecur.m_cCur != sRecur.m_cCurEnd)
        {
            auto* pPayload = reinterpret_cast<uintptr_t*>(
                reinterpret_cast<char*>(sRecur.DpInsertList) + sizeof(SBaseGeomListHeader));

            const auto pFirstGeom = reinterpret_cast<ZBaseGeom*>(pPayload[sRecur.m_cCur] & ~uintptr_t(7));
            const auto pLastGeom = reinterpret_cast<ZBaseGeom*>(pPayload[sRecur.m_cCur + 1] & ~uintptr_t(7));
            sRecur.m_cCur += 2;

            if (pDrawGeomsList >= pDrawGeomsListEnd)
            {
                break;
            }

            *pDrawGeomsList++ = pFirstGeom;
            *pDrawGeomsList++ = pLastGeom;

            sRecur.NextPair();
        }

        return pDrawGeomsList;
    }

    ZBaseGeom** ZROOM::GetStaticLightsRecur(ZBaseGeom** pDrawGeomsList, ZBaseGeom** pDrawGeomsListEnd)
    {
        for (auto* pBaseGeom = m_pGroupFirst; ForGroupsCheck(pBaseGeom); pBaseGeom = pBaseGeom->Next())
        {
            if (!pBaseGeom->IsDerivedFrom<ZROOM>())
            {
                auto* pGroup = reinterpret_cast<ZGROUP*>(pBaseGeom->GetGeom());
                pDrawGeomsList = pGroup->GetStaticLights(pDrawGeomsList, pDrawGeomsListEnd);
            }
        }

        return ZGROUP::GetStaticLights(pDrawGeomsList, pDrawGeomsListEnd);
    }

    bool ZROOM::NotInRoomTree() const
    {
        return (RoomControl() & ZROOM_NOT_IN_TREE) != 0;
    }

    void ZROOM::FreeGeomsLists()
    {
        FreeDynamicGeomList();
        REFTAB32 aCollected;

        {
            SGeomPairRecursion sRecur {};
            sRecur.InitPair(m_lStaticGeomsPrimDrawList);

            while (sRecur.DpInsertList && sRecur.m_cCur != sRecur.m_cCurEnd)
            {
                auto* pPayload = reinterpret_cast<uintptr_t*>(
                    reinterpret_cast<char*>(sRecur.DpInsertList) + sizeof(SBaseGeomListHeader));
                const uintptr_t lFirstGeom = pPayload[sRecur.m_cCur] & ~uintptr_t(7);
                const uintptr_t lLastGeom = pPayload[sRecur.m_cCur + 1] & ~uintptr_t(7);
                sRecur.m_cCur += 2;

                for (uintptr_t lGeom = lFirstGeom; lGeom <= lLastGeom; lGeom += sizeof(ZBaseGeom))
                    aCollected.Add(static_cast<uint32_t>(lGeom));

                sRecur.NextPair();
            }
        }

        {
            SGeomPairRecursion sRecur {};
            sRecur.InitPair(m_lStaticGeomsCustomDrawList);

            while (sRecur.DpInsertList && sRecur.m_cCur != sRecur.m_cCurEnd)
            {
                auto* pPayload = reinterpret_cast<uintptr_t*>(
                    reinterpret_cast<char*>(sRecur.DpInsertList) + sizeof(SBaseGeomListHeader));
                const uintptr_t lFirstGeom = pPayload[sRecur.m_cCur] & ~uintptr_t(7);
                const uintptr_t lLastGeom = pPayload[sRecur.m_cCur + 1] & ~uintptr_t(7);
                sRecur.m_cCur += 2;

                for (uintptr_t lGeom = lFirstGeom; lGeom <= lLastGeom; lGeom += sizeof(ZBaseGeom))
                    aCollected.Add(static_cast<uint32_t>(lGeom));

                sRecur.NextPair();
            }
        }

        {
            SGeomPairRecursion sRecur {};
            sRecur.InitPair(m_lDynamicGeomsDrawList);

            while (sRecur.DpInsertList && sRecur.m_cCur != sRecur.m_cCurEnd)
            {
                auto* pPayload = reinterpret_cast<uintptr_t*>(
                    reinterpret_cast<char*>(sRecur.DpInsertList) + sizeof(SBaseGeomListHeader));
                const uintptr_t lFirstGeom = pPayload[sRecur.m_cCur] & ~uintptr_t(7);
                const uintptr_t lLastGeom = pPayload[sRecur.m_cCur + 1] & ~uintptr_t(7);
                sRecur.m_cCur += 2;

                for (uintptr_t lGeom = lFirstGeom; lGeom <= lLastGeom; lGeom += sizeof(ZBaseGeom))
                    aCollected.Add(static_cast<uint32_t>(lGeom));

                sRecur.NextPair();
            }
        }

        for (auto* pBaseGeom : aCollected.As<ZBaseGeom*>())
            pBaseGeom->DetachFromRoomsDrawLists(this);
    }

    void ZROOM::FreeDynamicGeomList()
    {
        if (m_pDynamicGeoms)
        {
            for (auto* pBaseGeom : m_pDynamicGeoms->As<ZBaseGeom*>())
            {
                RemoveDynamicGeomFromRoom(pBaseGeom);
                pBaseGeom->RemoveFromRoomList(this);
            }

            ZROOM::s_DynamicGeomsAllocator.Free(m_pDynamicGeoms);
            m_pDynamicGeoms = nullptr;
        }
    }

    void ZROOM::SetAttachedRoom(ZROOM* pRoom)
    {
        if (!m_rAttachedDrawBaseGeoms[0])
        {
            m_rAttachedDrawBaseGeoms[0] = ZGeomBuffer::Instance().GeomPtrToRef(pRoom);
        }
        else if (!m_rAttachedDrawBaseGeoms[1])
        {
            m_rAttachedDrawBaseGeoms[1] = ZGeomBuffer::Instance().GeomPtrToRef(pRoom);
        }
        else
        {
            ZINFO("Too many rooms (more than %d) adopt visibility from %s", 2, pRoom->Name());
        }
    }

    void ZROOM::CalcBoundFromExit(const ZROOM::ZExit* pExit, ZMat3x3& mMat, ZVector3& vPos, ZVector3& vSize)
    {
        ZVector3 vEdge1 = pExit->p2 - pExit->p1;
        ZVector3 vEdge2 = pExit->p3 - pExit->p1;

        ZVector3 vNormal;
        vcross(vNormal.Get(), vEdge1.Get(), vEdge2.Get());
        vnorm(vNormal.Get());

        createmat(mMat.data, vNormal.Get(), nullptr);

        ZVector3 vMin { 0.0f, 0.0f, 0.0f };
        ZVector3 vMax { 0.0f, 0.0f, 0.0f };

        for (const ZVector3* pPoint = &pExit->p2; pPoint <= &pExit->p4; ++pPoint)
        {
            ZVector3 vPoint = *pPoint - pExit->p1;
            vmtmul(vPoint.Get(), mMat.data);

            vmin(vMin.Get(), vPoint.Get());
            vmax(vMax.Get(), vPoint.Get());
        }

        vPos = (vMax + vMin) * 0.5f;
        vSize = vMax - vPos;

        vmmul(vPos.Get(), mMat.data);
        vPos += pExit->p1;
    }

    uint32_t ZROOM::GetExitsIndicesInsideBox(uint16_t* pExitIndices, uint32_t maxCount, const ZMat3x3& mMat, const ZVector3& vPos, const ZVector3& vSize)
    {
        if (!m_lNrExits)
            return 0u;

        ZMat3x3 mRoot;
        ZVector3 vRoot;

        GetRootTM(mRoot, vRoot);

        uint32_t lResult = 0u;
        for (uint32_t lExitIdx = 0; lExitIdx < m_lNrExits; ++lExitIdx)
        {
            ZMat3x3 mExitMat;
            ZVector3 vExitPos;
            ZVector3 vExitSize;

            CalcBoundFromExit(&m_pExits[lExitIdx], mExitMat, vExitPos, vExitSize);

            vmmul(vExitPos.Get(), mRoot.data);
            vadd(vExitPos.Get(), vRoot.Get());

            if (rectBoxColi(mMat, vPos, vSize, mExitMat, vExitPos, vExitSize))
            {
                pExitIndices[lResult] = static_cast<uint16_t>(lExitIdx);
                ++lResult;
                if (lResult == maxCount)
                {
                    break;
                }
            }
        }

        return lResult;
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        static RTP::ZDataProperty<REFTAB*> NamespaceItem_2074
        {
            .m_Node = {
                .m_Next = nullptr,
                .m_Name = "m_pActorsAwareOfBrokenLight",
                .m_Filter = 2
            },
            .m_VirtualTable = &RTP::VirtualTables::Data_REFTAB_ptr,
            .m_Offset = CLASS_PROPERTY(ZROOM, m_pActorsAwareOfBrokenLight)
        };

        static RTP::ZDataProperty<REFTAB> NamespaceItem_2073
        {
            .m_Node = {
                .m_Next = NamespaceItem_2074,
                .m_Name = "m_FurnitureList",
                .m_Filter = 2
            },
            .m_VirtualTable = VirtualTable_DP__3,
            .m_Offset = CLASS_PROPERTY(ZROOM, m_FurnitureList)
        };

        static RTP::ZDataProperty<REFTAB> NamespaceItem_2072
        {
            .m_Node = {
                .m_Next = NamespaceItem_2073,
                .m_Name = "m_LightSwitches",
                .m_Filter = 2
            },
            .m_VirtualTable = VirtualTable_DP__3,
            .m_Offset = CLASS_PROPERTY(ZROOM, m_LightSwitches)
        };

        static RTP::ZDataProperty<uint32_t> NamespaceItem_2071
        {
            .m_Node = {
                .m_Next = NamespaceItem_2072,
                .m_Name = "m_iLightState",
                .m_Filter = 2
            },
            .m_VirtualTable = VirtualTable_DP__6,
            .m_Offset = CLASS_PROPERTY(ZROOM, m_iLightState)
        };

        static RTP::ZDataProperty<uint32_t> NamespaceItem_2070
        {
            .m_Node = {
                .m_Next = NamespaceItem_2071,
                .m_Name = "m_iLastVisibleFrameCount",
                .m_Filter = 2
            },
            .m_VirtualTable = VirtualTable_DP__6,
            .m_Offset = CLASS_PROPERTY(ZROOM, m_iLastVisibleFrameCount)
        };

        static RTP::ZDataProperty<ZAUDIOREF> NamespaceItem_2069
        {
            .m_Node = {
                .m_Next = NamespaceItem_2070,
                .m_Name = "m_dwRoomRef",
                .m_Filter = 3
            },
            .m_VirtualTable = VirtualTable_DP__21,
            .m_Offset = CLASS_PROPERTY(ZROOM, m_dwRoomRef)
        };

        static RTP::ZDataProperty<int> NamespaceItem_2068
        {
            .m_Node = {
                .m_Next = NamespaceItem_2069,
                .m_Name = "m_lSoundGraphId",
                .m_Filter = 2
            },
            .m_VirtualTable = VirtualTable_DP__7,
            .m_Offset = CLASS_PROPERTY(ZROOM, m_lSoundGraphId)
        };

        static RTP::ZDataProperty<uint8_t> NamespaceItem_2067
        {
            .m_Node = {
                .m_Next = NamespaceItem_2068,
                .m_Name = "m_lNrNeighborRooms",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_DP__32,
            .m_Offset = CLASS_PROPERTY(ZROOM, m_lNrNeighborRooms)
        };

        static RTP::ZDataProperty<uint32_t> NamespaceItem_2065
        {
            .m_Node = {
                .m_Next = NamespaceItem_2067,
                .m_Name = "m_lNrExits",
                .m_Filter = 1
            },
            .m_VirtualTable = VirtualTable_DP__6,
            .m_Offset = CLASS_PROPERTY(ZROOM, m_lNrExits)
        };
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZROOM,      // ClassName
        ZTreeGroup, // BaseClass
        0x0097B548, // OldClassInfoAddr
        "ZROOM", // FactoryName
        0x0076E998, // FactoryNameAddr
        cProperties::NamespaceItem_2065, // FirstProperty
        0x0080E35C, // PropertiesAddr
        0x0097B4D4, // IdAddr
        0x0097B4D8  // MaskAddr
    );
#   pragma endregion

    uint8_t ZROOM::s_DynamicGeomsBuffer[32768] { 0 }; // PC 0x009734C8
    ZPoolAllocator ZROOM::s_DynamicGeomsAllocator { (char*)&ZROOM::s_DynamicGeomsBuffer[0], 32768, "ZROOM::s_DynamicGeomsAllocator", false }; // PC 0x0097B4E4
}
