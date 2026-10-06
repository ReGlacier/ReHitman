#include <Glacier/Items/ZItemTemplate.h>

#include <Glacier/Items/ZItem.h>
#include <Glacier/Geom/ZBaseGeom.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Items/ZItemState.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/EventBase/ZEventBase.h>
#include <Glacier/Com/CCom.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZSTL/StringUtils.h>
#include <Glacier/Runtime/ZGEOMCLASSINFO.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Data/ZGameData.h>
#include <Glacier/System/ZSysInterface.h>


namespace Glacier
{
    ZItemTemplate::ZItemTemplate(const char* psName, ZBaseGeom* pBaseGeom)
        : ZGROUP(psName, pBaseGeom)
    {
        m_eItemHands = IH_NONE;
        m_eItemSize = eITEMSIZE_SMALL;
        m_rInventoryPicture = 0;
        m_rPointerPicture = 0;
        m_rPointerContextPicture = 0;

        m_pStates = REFTAB::MakeReftab(16, 1);

        m_msgSetItemState = g_pEngineData->RegisterZMsg("MSG_ITEMSETSTATE", 0, __FILE__, __LINE__);
        m_msgGetItemSettings = g_pEngineData->RegisterZMsg("MSG_ITEMGETANIMNAME", 0, __FILE__, __LINE__);

        m_rMaterial = 0;
        m_bSendImpactEvent = true;
    }

    ZItemTemplate::~ZItemTemplate()
    {
        if (m_pStates != nullptr)
            REFTAB::DeleteReftab(m_pStates);
    }

    const RTP::ZPropertyInfo& ZItemTemplate::GetProperties() const
    {
        return ZItemTemplate::Info;
    }

    uint32_t ZItemTemplate::GetObjectId() const
    {
        return ZItemTemplate::m_Id;
    }

    void ZItemTemplate::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZItemTemplate::m_Id;
        mask = ZItemTemplate::m_Mask;
    }

    ZGEOMCLASSINFO* ZItemTemplate::GetOldClassInfo() const
    {
        return ZItemTemplate::m_OldClassInfo;
    }

    void ZItemTemplate::ClassInit()
    {
        if ((m_baseGeom->m_lControl & 0x400) == 0)
            Delete();
    }

    void ZItemTemplate::CopyData(const ZGEOM* Source)
    {
        ZGROUP::CopyData(Source);

        if (Source->IsDerivedFrom<ZItemTemplate>())
        {
            const ZItemTemplate* pSrc = static_cast<const ZItemTemplate*>(Source);
            m_eItemHands = pSrc->m_eItemHands;
            m_eItemSize = pSrc->m_eItemSize;
            m_rInventoryPicture = pSrc->m_rInventoryPicture;
            m_rPointerPicture = pSrc->m_rPointerPicture;
            m_rPointerContextPicture = pSrc->m_rPointerContextPicture;
            m_rMaterial = pSrc->m_rMaterial;
            m_bSendImpactEvent = pSrc->m_bSendImpactEvent;
        }
    }

    ZItem* ZItemTemplate::CreateItem(ZGROUP* pGroup, unsigned int iGeomResourceId, bool bOverrideVisibleForNPC, bool bVisibleForNPC)
    {
        const uint32_t lClassId = GetItemClassId();
        ZGEOMCLASSINFO* pClassInfo = g_pEngineData->GetGeomClassInfo(lClassId);
        if (pClassInfo == nullptr)
            ZASSERT(false);

        const char* pszName = m_baseGeom ? m_baseGeom->m_Name : nullptr;
        if (pszName == nullptr)
            pszName = "<NONAME>";

        ZItem* pItem = static_cast<ZItem*>(
            pGroup->CreateResourceGeom(pszName, iGeomResourceId, pClassInfo->Type(), true));
        if (pItem == nullptr)
        {
            ZASSERT(false);
            return nullptr;
        }

        pItem->SetItemTemplate(GetRef());
        pItem->DoInit();

        if (bOverrideVisibleForNPC)
            pItem->SetVisibleToNPCs(bVisibleForNPC);
        else if (m_bIsVisibleToNPCs)
            pItem->SetVisibleToNPCs(m_bIsVisibleToNPCs);

        g_pGameData->AddItemOnGround(pItem);
        return pItem;
    }

    ZItem* ZItemTemplate::CreateItemAndActuallyUseDestinationParameter(ZGROUP* pGroup, unsigned int iGeomResourceId, bool bOverrideVisibleForNPC, bool bVisibleForNPC)
    {
        return CreateItem(pGroup, iGeomResourceId, bOverrideVisibleForNPC, bVisibleForNPC);
    }

    uint32_t ZItemTemplate::GetItemClassId() const
    {
        return 0x1007D1u;
    }

    void ZItemTemplate::StateNotify(ZGEOM* pGeom, int lStates)
    {
        // Each set bit in lStates appends a (state, geom-ref) record to m_pStates.
        // REFTAB::Add stores the state value in entry[0] and returns &entry[1].
        for (int i = 0; i != 128; ++i)
        {
            if (lStates & 1)
            {
                uint32_t* pRef = m_pStates->Add(static_cast<uint32_t>(1u << i));
                *pRef = pGeom->GetRef();
            }

            lStates >>= 1;
            if (lStates == 0)
                break;
        }
    }

    void ZItemTemplate::SetStates(CCom* pCom)
    {
        int lState = 0;
        pCom->GetVal("lState", &lState);

        if (m_pStates == nullptr)
            return;

        RefRun Run;
        m_pStates->RunInitNxtRef(&Run);
        for (uint32_t* pEntry = m_pStates->RunNxtRefPtr(&Run); pEntry != nullptr; pEntry = m_pStates->RunNxtRefPtr(&Run))
        {
            if (static_cast<int>(pEntry[0]) == lState)
            {
                ZGEOM* pGeom = ZGEOM::RefToPtr(pEntry[1]);
                if (pGeom != nullptr)
                {
                    ZEventBase* pEvent = pGeom->FindEvent("ItemState*");
                    if (pEvent != nullptr)
                        pEvent->Call(0x20, pCom, m_msgGetItemSettings);
                }
                else
                {
                    m_pStates->RunDelRef(&Run);
                }
            }
        }
    }

    void ZItemTemplate::ModifyState(CCom* /*pCom*/)
    {
        // PC resolves this vtable slot to the shared do-nothing "return 1" stub.
    }

    void ZItemTemplate::SetStateGeometry(ZItem* pItem, ZGEOM* pGeom)
    {
        ZEventBase* pEvent = pGeom->FindEvent("ItemState*");
        if (pEvent != nullptr)
            pEvent->Call(0x20, pItem, m_msgSetItemState);
    }

    REFTAB* ZItemTemplate::GetAvailableStates() const
    {
        REFTAB* pResult = REFTAB::MakeReftab(32, 0);

        uint32_t lSeen = 0;
        if (m_pStates != nullptr)
        {
            RefRun Run;
            m_pStates->RunInitNxtRef(&Run);
            for (uint32_t i = m_pStates->RunNxtRef(&Run); Run._RunPtr != nullptr; i = m_pStates->RunNxtRef(&Run))
            {
                if (((1u << i) & lSeen) == 0)
                {
                    lSeen |= (1u << i);
                    pResult->Add(i);
                }
            }
        }

        return pResult;
    }

    REFTAB* ZItemTemplate::GetStates() const
    {
        return m_pStates;
    }

    bool ZItemTemplate::CheckStateExists(ITEMSTATE eState, const char* pszStateName)
    {
        if (m_pStates == nullptr)
            return false;

        RefRun Run;
        m_pStates->RunInitNxtRef(&Run);
        for (uint32_t* pEntry = m_pStates->RunNxtRefPtr(&Run); pEntry != nullptr; pEntry = m_pStates->RunNxtRefPtr(&Run))
        {
            if (static_cast<ITEMSTATE>(pEntry[0]) == eState)
            {
                if (pszStateName == nullptr)
                    return true;

                ZGEOM* pGeom = ZGEOM::RefToPtr(pEntry[1]);
                if (pGeom != nullptr)
                {
                    const char* pszGeomName = pGeom->m_baseGeom->m_Name;
                    if (pszGeomName == nullptr)
                        pszGeomName = "<NONAME>";
                    if (strcasecmp(pszStateName, pszGeomName) == 0)
                        return true;
                }
            }
        }

        return false;
    }

    void ZItemTemplate::FindStateGeoms(REFTAB* pRefTab, ITEMSTATE eState, const char* pszStateName)
    {
        if (m_pStates == nullptr)
            return;

        RefRun Run;
        m_pStates->RunInitNxtRef(&Run);
        for (uint32_t* pEntry = m_pStates->RunNxtRefPtr(&Run); pEntry != nullptr; pEntry = m_pStates->RunNxtRefPtr(&Run))
        {
            if (static_cast<ITEMSTATE>(pEntry[0]) == eState)
            {
                ZGEOM* pGeom = ZGEOM::RefToPtr(pEntry[1]);
                if (pGeom != nullptr)
                {
                    bool bMatch = true;
                    if (pszStateName != nullptr)
                    {
                        const char* pszGeomName = pGeom->m_baseGeom->m_Name;
                        if (pszGeomName == nullptr)
                            pszGeomName = "<NONAME>";
                        bMatch = (striwcmp(pszGeomName, pszStateName) == 0);
                    }

                    if (bMatch)
                        pRefTab->Add(pEntry[1]);
                }
                else
                {
                    m_pStates->RunDelRef(&Run);
                }
            }
        }
    }

    void ZItemTemplate::FindMainState(REFTAB* pRefTab)
    {
        if (m_pStates == nullptr)
            return;

        RefRun Run;
        m_pStates->RunInitNxtRef(&Run);
        for (uint32_t* pEntry = m_pStates->RunNxtRefPtr(&Run); pEntry != nullptr; pEntry = m_pStates->RunNxtRefPtr(&Run))
        {
            ZGEOM* pGeom = ZGEOM::RefToPtr(pEntry[1]);
            if (pGeom != nullptr)
            {
                ZEventBase* pEvent = pGeom->FindEvent("ItemState*");
                if (pEvent != nullptr && static_cast<ZItemState*>(pEvent)->IsMain())
                    pRefTab->Add(pEntry[1]);
            }
            else
            {
                m_pStates->RunDelRef(&Run);
            }
        }
    }

    ITEMHANDS ZItemTemplate::GetItemHands() const
    {
        return m_eItemHands;
    }

    void ZItemTemplate::SetItemHands(ITEMHANDS eHands)
    {
        m_eItemHands = eHands;
    }

    ITEMSIZE ZItemTemplate::GetItemSize() const
    {
        return m_eItemSize;
    }

    ZGEOM* ZItemTemplate::GetMainPos() const
    {
        ZItemTemplate* pSelf = const_cast<ZItemTemplate*>(this);
        ZGEOM* pGeom = pSelf->FindGeom("PosBox_Hand", nullptr);
        return pGeom ? pGeom : static_cast<ZGEOM*>(pSelf);
    }

    ZGEOM* ZItemTemplate::GetCenterPos() const
    {
        ZItemTemplate* pSelf = const_cast<ZItemTemplate*>(this);
        ZGEOM* pGeom = pSelf->FindGeom("PosBox_Center", nullptr);
        return pGeom ? pGeom : static_cast<ZGEOM*>(pSelf);
    }

    ZGEOM* ZItemTemplate::GetCameraPos() const
    {
        return const_cast<ZItemTemplate*>(this)->FindGeom("PosBox_Camera", nullptr);
    }

    uint32_t ZItemTemplate::GetMaterial() const
    {
        return m_rMaterial;
    }

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        static ZEnumEntry ItemHandsEntries[] = {
            {nullptr, IH_NONE, "IH_NONE"},
            {&ItemHandsEntries[0], IH_ONEHANDED, "IH_ONEHANDED"},
            {&ItemHandsEntries[1], IH_TWOHANDED, "IH_TWOHANDED"},
            {&ItemHandsEntries[2], IH_FORCE32, "IH_FORCE32"}};
        static ZEnumInfo ItemHandsInfo{&ItemHandsEntries[3], "ITEMHANDS", sizeof(ITEMHANDS)};

        static ZEnumEntry ItemSizeEntries[] = {
            {nullptr, eITEMSIZE_SMALL, "ITEMSIZE_SMALL"},
            {&ItemSizeEntries[0], ITEMSIZE_LARGE, "ITEMSIZE_LARGE"},
            {&ItemSizeEntries[1], ITEMSIZE_FORCE32, "ITEMSIZE_FORCE32"}};
        static ZEnumInfo ItemSizeInfo{&ItemSizeEntries[2], "ITEMSIZE", sizeof(ITEMSIZE)};

        // Chain is declared tail-first; head (m_eItemHands) is passed to DECLARE_GEOM_CLASS_IMPL.
        static RTP::ZDataProperty<bool> SendImpactEvent{
            .m_Node = {.m_Next = nullptr, .m_Name = "m_bSendImpactEvent", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Data_bool,
            .m_Offset = CLASS_PROPERTY(ZItemTemplate, m_bSendImpactEvent)};

        static RTP::ZDataProperty<uint> Material{
            .m_Node = {.m_Next = SendImpactEvent, .m_Name = "m_rMaterial", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Data_uint,
            .m_Offset = CLASS_PROPERTY(ZItemTemplate, m_rMaterial)};

        static RTP::ZDataProperty<REFTAB*> States{
            .m_Node = {.m_Next = Material, .m_Name = "m_pStates", .m_Filter = 2},
            .m_VirtualTable = &RTP::VirtualTables::Data_REFTAB_ptr,
            .m_Offset = CLASS_PROPERTY(ZItemTemplate, m_pStates)};

        static RTP::ZDataProperty<bool> IsVisibleToNPCs{
            .m_Node = {.m_Next = States, .m_Name = "m_bIsVisibleToNPCs", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Data_bool,
            .m_Offset = CLASS_PROPERTY(ZItemTemplate, m_bIsVisibleToNPCs)};

        static RTP::ZEnumProperty ItemSize{
            .m_Node = {.m_Next = IsVisibleToNPCs, .m_Name = "m_eItemSize", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Enum,
            .m_Offset = CLASS_PROPERTY(ZItemTemplate, m_eItemSize),
            .m_Info = &ItemSizeInfo};

        static RTP::ZEnumProperty ItemHands{
            .m_Node = {.m_Next = ItemSize, .m_Name = "m_eItemHands", .m_Filter = 1},
            .m_VirtualTable = &RTP::VirtualTables::Enum,
            .m_Offset = CLASS_PROPERTY(ZItemTemplate, m_eItemHands),
            .m_Info = &ItemHandsInfo};
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZItemTemplate, // ClassName
        ZGROUP, // BaseClass
        0x0099BFD8, // OldClassInfoAddr
        "ZItemTemplate", // FactoryName
        0x00773E90, // FactoryNameAddr
        cProperties::ItemHands, // FirstProperty
        0x0080C368, // PropertiesAddr
        0x0099BF30, // IdAddr
        0x0099BF34 // MaskAddr
    );
#   pragma endregion
}
