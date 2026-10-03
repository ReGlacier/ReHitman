#include <Glacier/Items/ZItem.h>

#include <Glacier/Items/ZItemTemplate.h>
#include <Glacier/Com/CCom.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/GameBase/ZCheckVisible.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Data/ZGameData.h>
#include <Glacier/RTP/Base.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZSTL/ZPoolAllocRefTab.h>
#include <Glacier/ZSTL/ZPoolAllocLinkSortRefTab.h>


namespace Glacier
{
    // Shared pool allocator backing every ZItem's four state/activation ref tables.
    // PC binary uses a single global ZPoolAllocator (stru_99C468); its buffer size is
    // runtime-initialized on PC so it cannot be read from the image.
    // TODO: Finish me - confirm the exact pool buffer size for stru_99C468 (e.g. via PS2).
    static char s_ZItemPoolBuffer[0x4000];
    static ZPoolAllocator s_ZItemPool(s_ZItemPoolBuffer, sizeof(s_ZItemPoolBuffer), "ZItem", false);

    // vtbl slot - (constructor)  PC 0x50EAD0
    ZItem::ZItem(const char* psName, ZBaseGeom* pBaseGeom)
        : ZGROUP(psName, pBaseGeom)
    {
        m_NewItem = true;
        m_iVisionID = 0xFF;           // "no vision id assigned" sentinel (low byte 0xFF)
        m_lCurrentState = eIS_NORMAL;
        m_rItemOwner = 0;
        m_rItemTemplate = 0;
        m_rMain = 0;

        // Pool-backed ref tables: two REFTAB-style and two linksort-style, all from the shared
        // ZItem pool allocator.
        m_pStateRemove = s_ZItemPool.Alloc<ZPoolAllocRefTab>(&s_ZItemPool, 8, 0);
        m_pStateReuse = s_ZItemPool.Alloc<ZPoolAllocRefTab>(&s_ZItemPool, 8, 0);
        m_pDeactivateStates = s_ZItemPool.Alloc<ZPoolAllocLinkSortRefTab>(&s_ZItemPool, 8, 0);
        m_pActivateStates = s_ZItemPool.Alloc<ZPoolAllocLinkSortRefTab>(&s_ZItemPool, 8, 0);

        m_msgSetItemState = g_pEngineData->RegisterZMsg("MSG_ITEMSETSTATE", 0, __FILE__, __LINE__);
        m_msgGetAvailableStates = g_pEngineData->RegisterZMsg("MSG_GETAVAILABLEITEMSTATES", 0, __FILE__, __LINE__);
        m_msgSetItem = g_pEngineData->RegisterZMsg("MSG_SetItem", 0, __FILE__, __LINE__);

        g_pGameData->AddItemOnGround(this);
    }

    // vtbl slot 0 (destructor body; PC deleting dtor 0x510ED0 -> body 0x50ECD0)
    ZItem::~ZItem()
    {
        if (g_pGameData != nullptr)
            g_pGameData->RemoveItemOnGround(this);

        // Delete any geometry still referenced by the state-remove table.
        if (m_pStateRemove != nullptr)
        {
            RefRun Run;
            m_pStateRemove->RunInitNxtRef(&Run);
            for (uint32_t* pEntry = m_pStateRemove->RunNxtRefPtr(&Run);
                 pEntry != nullptr;
                 pEntry = m_pStateRemove->RunNxtRefPtr(&Run))
            {
                ZGEOM* pGeom = ZGEOM::RefToPtr(*pEntry);
                if (pGeom != nullptr)
                    pGeom->Delete();
            }
        }

        // Tear down the four pool tables (Clear + destructor + release header to allocator).
        if (m_pStateReuse != nullptr)
        {
            m_pStateReuse->Clear();
            m_pStateReuse->~ZPoolAllocRefTab();
            s_ZItemPool.Free(m_pStateReuse);
        }
        if (m_pStateRemove != nullptr)
        {
            m_pStateRemove->Clear();
            m_pStateRemove->~ZPoolAllocRefTab();
            s_ZItemPool.Free(m_pStateRemove);
        }
        if (m_pActivateStates != nullptr)
        {
            m_pActivateStates->Clear();
            m_pActivateStates->~ZPoolAllocLinkSortRefTab();
            s_ZItemPool.Free(m_pActivateStates);
        }
        if (m_pDeactivateStates != nullptr)
        {
            m_pDeactivateStates->Clear();
            m_pDeactivateStates->~ZPoolAllocLinkSortRefTab();
            s_ZItemPool.Free(m_pDeactivateStates);
        }
    }

    // vtbl slot 12 (RTTI)  PC 0x50ECB0
    const RTP::ZPropertyInfo& ZItem::GetProperties() const
    {
        return ZItem::Info;
    }

    // vtbl slot 13 (RTTI)  PC 0x510A60
    uint32_t ZItem::GetObjectId() const
    {
        return ZItem::m_Id;
    }

    // vtbl slot 14 (RTTI)  PC 0x510A70
    void ZItem::GetObjectIdAndMask(uint32_t& id, uint32_t& mask) const
    {
        id = ZItem::m_Id;
        mask = ZItem::m_Mask;
    }

    // vtbl slot 15 (RTTI)  PC 0x50ECC0
    ZGEOMCLASSINFO* ZItem::GetOldClassInfo() const
    {
        return ZItem::m_OldClassInfo;
    }

    // vtbl slot 84  PC 0x510F70
    void ZItem::PostClassInit()
    {
        ZItemTemplate* pTemplate = GetItemTemplate();
        ZASSERT(pTemplate != nullptr);
        if (pTemplate == nullptr)
        {
            MakeInactive();
            return;
        }

        VerifyItemTemplate(pTemplate);

        CCom ccomLocal;
        ccomLocal.SetVal("lState", static_cast<int>(m_lCurrentState));
        ClassCommand(m_msgSetItemState, &ccomLocal);

        // Re-bind the owner. The stored owner ref is taken and cleared so SetItemOwner
        // sees the new value cleanly.
        const bool bIsNew = m_NewItem;
        const uint32_t rOwner = m_rItemOwner;
        ZGROUP* pParentGroup = nullptr;
        m_rItemOwner = 0;

        if (bIsNew)
        {
            m_NewItem = false;
            CreateFromTemplate();
            SetItemOwner(rOwner, nullptr, true, true);
        }
        else
        {
            // TODO: Finish me after ZLNKOBJ::ParentGroup path verified.
            // PC: when owner ref set and not ZCOWNERDRAW, if RefToPtr(owner) is a ZLNKOBJ
            // then parent group becomes ZBaseGeom::ParentGroup(that zlnkobj's base geom).
            SetItemOwner(rOwner, pParentGroup, false, true);
        }

        g_pEngineData->UnlockMinMax();
        CalcCenSize();
        g_pEngineData->LockMinMax();
    }

    // vtbl slot 104  PC 0x510F30
    void ZItem::CopyData(const ZGEOM* Source)
    {
        ZGROUP::CopyData(Source);

        if (Source->IsDerivedFrom<ZItem>())
        {
            const ZItem* pSrc = static_cast<const ZItem*>(Source);
            m_rItemTemplate = pSrc->m_rItemTemplate;
            m_rItemOwner = pSrc->m_rItemOwner;
        }
    }

    // Non-virtual engine method  PC 0x511630
    bool ZItem::SetVisibleToNPCs(bool isVisible)
    {
        m_bVisibleToNPCs = isVisible;

        if (isVisible)
        {
            if (m_rItemOwner == 0)
                ZCheckVisible::m_pCheckVisible->UpdateSeeableItem(this);
        }
        else if (static_cast<uint8_t>(m_iVisionID) != 0xFF)
        {
            ZCheckVisible::m_pCheckVisible->RemoveSeeableItem(this);
        }

        return m_bVisibleToNPCs;
    }

    // vtbl slot 147  PC sub_5110F0
    void ZItem::CreateFromTemplate()
    {
        // TODO: Finish me. PC 0x5110F0 builds the concrete item geometry from the bound
        // ZItemTemplate. Depends on the template state-geom creation path; left out to keep
        // the vtable complete until the full logic is reversed.
    }

    // vtbl slot 148  PC 0x511210
    void ZItem::GetItemRootTM(float* mat, float* pos)
    {
        // Forwards to GetMainItemRootTM (PC tail-calls vtbl slot 149 with the same args).
        GetMainItemRootTM(mat, pos);
    }

    // vtbl slot 149  PC 0x511220
    void ZItem::GetMainItemRootTM(float* mat, float* pos)
    {
        // TODO: Finish me. PC 0x511220 selects the root transform from the main geom /
        // container / link-object hierarchy. The owner branch needs ZItemContainer class-info
        // (m_Id/m_Mask), which is not declared in the project yet.
        (void)mat;
        (void)pos;
    }

    // vtbl slot 150  PC 0x6D1230 (COMDAT-folded with ZItemTemplate::GetItemHands)
    ITEMSTATE ZItem::GetState() const
    {
        return m_lCurrentState;
    }

    // vtbl slot 151  PC 0x514000
    bool ZItem::SetState(ITEMSTATE eState, CCom* pCom)
    {
        if (eState == m_lCurrentState)
            return true;

        // Bit 10 of the base-geom control flags (ZCINACTIVE-like gate) selects active/inactive.
        const uint32_t lActive = ~static_cast<uint32_t>(m_baseGeom->m_lControl >> 10);
        if (eState == IS_HIDE)
        {
            if ((lActive & 1u) != 0)
                MakeInactive();
        }
        else if ((lActive & 1u) == 0)
        {
            MakeActive();
        }

        ZGEOM* pTemplate = ZGEOM::RefToPtr(m_rItemTemplate);
        if (pTemplate == nullptr)
            return false;

        if (!static_cast<ZItemTemplate*>(pTemplate)->CheckStateExists(eState, nullptr))
            return false;

        CCom ccomLocal;
        CCom* pUse = (pCom != nullptr) ? pCom : &ccomLocal;
        pUse->SetVal("lState", static_cast<int>(eState));
        ClassCommand(m_msgSetItemState, pUse);
        return true;
    }

    // vtbl slot 152  PC 0x50F0A0
    void ZItem::Place(const ZMat3x3& mMat, const ZVector3& vPos)
    {
        // TODO: Finish me. PC 0x50F0A0 composes the main geom's root transform with mMat and
        // re-positions this item (GetMain -> GetMatPos -> matrix mul -> SetMatPos). Needs the
        // project ZMat3x3/ZVector3 math helpers verified before transcribing the multiply order.
        (void)mMat;
        (void)vPos;
    }

    // vtbl slot 153  PC 0x50F070
    uint32_t ZItem::SetMain(uint32_t rMain)
    {
        const uint32_t rOld = m_rMain;
        m_rMain = rMain;
        return rOld;
    }

    // vtbl slot 154  PC 0x50F080
    ZGEOM* ZItem::GetMain()
    {
        if (m_rMain == 0)
            return nullptr;
        return ZGEOM::RefToPtr(m_rMain);
    }

    // vtbl slot 155  PC 0x511760
    void ZItem::GetMainMatPos(float* mat, float* pos, uint32_t lBoneId)
    {
        // TODO: Finish me. PC 0x511760 resolves a (matrix, position) pair for the main item on a
        // bone. Heavy math + template position-geom lookup; left as a stub until verified.
        (void)mat;
        (void)pos;
        (void)lBoneId;
    }

    // vtbl slot 156  PC 0x511670
    void ZItem::SetItemTemplate(Glacier::ZREF itemTemplateRef)
    {
        uint32_t rNew = itemTemplateRef;
        if (itemTemplateRef != 0)
        {
            ZGEOM* pCandidate = ZGEOM::RefToPtr(itemTemplateRef);
            if (pCandidate == nullptr ||
                (pCandidate->GetObjectId() & ZItemTemplate::m_Mask) != ZItemTemplate::m_Id)
            {
                rNew = 0;
            }
        }

        // TODO: Finish me. PC also deletes the geometry currently referenced by
        // m_pStateRemove before clearing it. Kept minimal to avoid the pool-iteration risk.

        m_pStateRemove->Clear();
        m_rItemTemplate = rNew;
        SetState(eIS_NORMAL, nullptr);
    }

    // vtbl slot 157  PC 0x50F060
    ZItemTemplate* ZItem::GetItemTemplate()
    {
        return reinterpret_cast<ZItemTemplate*>(ZGEOM::RefToPtr(m_rItemTemplate));
    }

    // vtbl slot 158  PC 0x511730
    void ZItem::VerifyItemTemplate(ZItemTemplate const* pTemplate)
    {
        // PC asserts the pointer is a valid ZItemTemplate. The engine relies on the caller;
        // exposed here as a verification hook (returns nothing, matches the project slot).
        (void)pTemplate;
    }

    // vtbl slot 159  PC 0x5113E0
    void ZItem::SetItemOwner(uint32_t rOwner, ZGROUP* pGroup, bool b3, bool b4)
    {
        // TODO: Finish me. PC 0x5113E0 re-parents the item, sends the inventory add/remove
        // messages, toggles ground registration and ZCheckVisible, and selects the attach
        // parent. Several branches depend on ZItemContainer class-info and ZItem message
        // statics that are not yet declared (see task log NEEDS USER INPUT).
        (void)rOwner;
        (void)pGroup;
        (void)b3;
        (void)b4;
    }

    // vtbl slot 160  PC 0x50F040
    ZGEOM* ZItem::GetItemOwner() const
    {
        if (m_rItemOwner == 0)
            return nullptr;
        return ZGEOM::RefToPtr(m_rItemOwner);
    }

    // vtbl slot 161  PC 0x50EF60
    void ZItem::GetAction(uint32_t lIndex)
    {
        // TODO: Finish me after ZAction reversed. PC 0x50EF60 does
        //   ZAction* p = (ZAction*)FindEvent("Action");
        //   return p ? p->GetSubAction(0, 0, lIndex, 0) : nullptr;
        // ZAction has no project header yet, so this returns nothing.
        (void)lIndex;
    }

    // vtbl slot 162  PC 0x5157B0
    void* ZItem::InitPickup()
    {
        // TODO: Finish me after ZAction / pickup rout reversed. PC 0x5157B0 wires the pickup
        // action (a "Pickup" rout event on the bound template) and caches the action pointer.
        return nullptr;
    }

    // vtbl slot 163  PC 0x50EF90
    void ZItem::EnablePickup(bool bEnable)
    {
        // TODO: Finish me after ZAction reversed. PC 0x50EF90 resolves the template's pickup
        // action via GetAction and calls ZAction::Show/Hide based on bEnable.
        (void)bEnable;
    }

    // vtbl slot 164  PC 0x50EFE0
    void ZItem::OnMoved()
    {
        if (!m_bVisibleToNPCs)
            return;

        if (static_cast<uint8_t>(m_iVisionID) != 0xFF)
        {
            ZCheckVisible::m_pCheckVisible->UpdateSeeableItem(this);
            m_bInMotion = false;
        }
    }

    // vtbl slot 165  PC 0x50F010
    void ZItem::OnMoving()
    {
        if (!m_bVisibleToNPCs)
            return;

        if (static_cast<uint8_t>(m_iVisionID) != 0xFF)
        {
            m_bInMotion = true;
            // TIMETYPE.secs is fixed-point at 1024 ticks/second; convert to seconds.
            m_fLastUpdatedPosition = static_cast<float>(g_pSysInterface->FrameTime.secs) * (1.0f / 1024.0f);
        }
    }

    // vtbl slot 166  PC 0x50EDD0
    void ZItem::Delete()
    {
        if (static_cast<uint8_t>(m_iVisionID) != 0xFF)
            ZCheckVisible::m_pCheckVisible->RemoveSeeableItem(this);

        ZGEOM::Delete();
    }

    // vtbl slot 167  PC 0x5119E0
    void ZItem::Clear(uint32_t typeId)
    {
        // TODO: Finish me. PC 0x5119E0 destroys all child base-geoms, then (when typeId is
        // non-zero) recreates a single "NewObject" child via ZGROUP::CreateGeom. Requires the
        // ZGROUP children teardown (ZBaseGeom::Dtor + free) verified against the project build.
        (void)typeId;
    }

    // vtbl slot 168  PC 0x511920
    ZGEOM* ZItem::GetMarkedGeom(char const* pszName)
    {
        // TODO: Finish me. PC 0x511920 scans m_pStateReuse for a geometry whose base-geom name
        // matches pszName (case-insensitive), moves its ref out of reuse and into the
        // remove/deactivate tables, and returns it.
        (void)pszName;
        return nullptr;
    }

    // vtbl slot 169  PC 0x50F170
    void ZItem::AddActivate(ZItemState* pItemState, float fDelay)
    {
        // TODO: Finish me. PC 0x50F170 enables class call 16 and schedules the state in the
        // activate linksort table (m_pActivateStates->AddSort(...)).
        (void)pItemState;
        (void)fDelay;
    }

    // vtbl slot 170  PC 0x50F140
    void ZItem::AddDeactivate(uint32_t rState, float fDelay)
    {
        // TODO: Finish me. PC 0x50F140 enables class call 16 and schedules the state in the
        // deactivate linksort table (m_pDeactivateStates->AddSort(...)).
        (void)rState;
        (void)fDelay;
    }

    // vtbl slot 171  PC 0x511310
    void ZItem::UpdateActivate()
    {
        // TODO: Finish me. PC 0x511310 pops due entries from m_pActivateStates (time compare vs
        // frame time), resolves the scheduled event (ZEventBuffer) and re-positions the item.
    }

    // vtbl slot 172  PC 0x50EE20
    void ZItem::UpdateDeactivate()
    {
        // TODO: Finish me. PC 0x50EE20 pops due entries from m_pDeactivateStates, moves each ref
        // into the reuse/remove tables and deletes the geometry.
    }

#   pragma region " --- RTTI --- "
    // TODO: Finish me after the RTP property chain (head PC 0x0080C374, tail ..0x0080C400) is
    // reversed: ZItem serializes m_lCurrentState, m_rItemTemplate, m_bVisibleToNPCs,
    // m_rItemOwner and m_msgSetItem. PC property names are runtime-initialized; recover them from
    // PS2 / .rdata and pass the head node instead of nullptr (mirrors the accepted ZCAMERA /
    // ZItemTemplate pass-2 precedent).
    DECLARE_GEOM_CLASS_IMPL(
        ZItem,           // ClassName
        ZGROUP,          // BaseClass
        0x0099C118,      // OldClassInfoAddr (dword_99C118)
        "ZItem",         // FactoryName
        0x0,             // FactoryNameAddr (documentation only; not bound by the macro)
        nullptr,         // FirstProperty (TODO: real chain head)
        0x0080C418,      // PropertiesAddr (ZItem::Info)
        0x0099BF50,      // IdAddr (ZItem::m_Id)
        0x0099BF54       // MaskAddr (ZItem::m_Mask)
    );
#   pragma endregion
}
