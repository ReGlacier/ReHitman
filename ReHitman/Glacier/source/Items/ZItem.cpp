#include <Glacier/Items/ZItemState.h>

#include <Glacier/ZSTL/StringUtils.h>
#include <Glacier/Items/ZItem.h>

#include <Glacier/EventBase/ZEventBuffer.h>
#include <Glacier/Items/ZItemTemplate.h>
#include <Glacier/Items/ZItemContainer.h>
#include <Glacier/IK/ZLNKOBJ.h>
#include <Glacier/IK/ZBoneModifyBase.h>
#include <Glacier/ZAction.h>
#include <Glacier/Com/CCom.h>
#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/GameBase/ZCheckVisible.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Data/ZGameData.h>
#include <Glacier/RTP/Base.h>
#include <Glacier/RTP/PropertyTypes.h>
#include <Glacier/RTP/VirtualTables.h>
#include <Glacier/Geom/ExGeomData.h>
#include <Glacier/Serializer/ISerializerStream.h>
#include <Glacier/ZSTL/REFTAB.h>
#include <Glacier/ZSTL/ZPoolAllocRefTab.h>
#include <Glacier/ZSTL/ZPoolAllocLinkSortRefTab.h>


namespace Glacier
{
    // PC 0x73D3E0 initializes the shared allocator at 0x99C468 with a 96 KiB buffer.
    // All ZItem state and activation tables use this pool.
    static char s_ZItemPoolBuffer[0x18000];
    static ZPoolAllocator s_ZItemPool(s_ZItemPoolBuffer, sizeof(s_ZItemPoolBuffer), "ZItem::s_ItemMemoryAllocator", false);

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
            if (rOwner != 0 && (m_baseGeom->m_lControl & 0x200000u) == 0)
            {
                ZGEOM* pPossibleZlnkObj = ZGEOM::RefToPtr(rOwner);
                if (geom_cast<ZLNKOBJ>(pPossibleZlnkObj) != nullptr)
                    pParentGroup = pPossibleZlnkObj->BaseGeom()->ParentGroup();
            }
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
        ZItemTemplate* itemTemplate = GetItemTemplate();
        if (itemTemplate == nullptr)
            return;

        REFTAB* states = m_pActivateStates;
        ZGEOM* main = GetMain();
        if (states != nullptr && ZEventBuffer::m_Instance != nullptr)
        {
            RefRun run;
            states->RunInitNxtRef(&run);
            for (uint32_t* entry = states->RunNxtRefPtr(&run); entry != nullptr;
                 entry = states->RunNxtRefPtr(&run))
            {
                ZEventBase* stateEvent = ZEventBuffer::m_Instance->ConvEventRefToPtr(*entry);
                if (stateEvent == nullptr || stateEvent->m_pBaseGeom == nullptr)
                    continue;

                if (main != nullptr)
                    itemTemplate->SetStateGeometry(this, stateEvent->m_pBaseGeom);
            }
        }

        if (main == nullptr)
        {
            ZGEOM* ground = FindGeom("Ground*", nullptr);
            if (ground != nullptr)
                SetMain(ground->GetRef());
            else
            {
                ZGEOM* handPosition = FindGeom("PosBox_Hand", nullptr);
                if (handPosition != nullptr)
                    SetMain(handPosition->GetRef());
            }
        }
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
        ZBaseGeom* pBaseGeom = m_baseGeom;
        if ((pBaseGeom->m_lControl & 0x200000u) != 0)
        {
            ZGEOM* pMain = GetMain();
            if (pMain != nullptr)
            {
                ZBaseGeom* pMainBaseGeom = pBaseGeom;
                if (pMain->IsDerivedFrom<ZItemContainer>())
                {
                    pMain = static_cast<ZItemContainer*>(pMain)->GetMain();
                    pMainBaseGeom = pMain != nullptr ? pMain->BaseGeom() : pBaseGeom;
                }

                const ZLNKOBJ* pLinkObject = geom_cast<ZLNKOBJ>(pMain);
                ZASSERT(pLinkObject != nullptr);
                if (pLinkObject != nullptr)
                {
                    ZMat3x3 mBounds;
                    ZVector3 vCenter;
                    ZVector3 vSize;
                    pMain->ExpandBounds(mBounds, vCenter, vSize, pMainBaseGeom);
                    pLinkObject->GetRootMatPos(*reinterpret_cast<ZMat3x3*>(mat),
                                                *reinterpret_cast<ZVector3*>(pos));
                }
            }
        }
        else
        {
            ZGROUP* pParentGroup = pBaseGeom->ParentGroup();
            if (pParentGroup != nullptr && pParentGroup->IsDerivedFrom<ZItemContainer>())
            {
                pParentGroup->GetRootTM(*reinterpret_cast<ZMat3x3*>(mat),
                                        *reinterpret_cast<ZVector3*>(pos));
            }
            else
            {
                GetRootTM(*reinterpret_cast<ZMat3x3*>(mat), *reinterpret_cast<ZVector3*>(pos));
            }
        }
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
        ZGEOM* pMain = GetMain();
        if (pMain == nullptr)
            return;

        ZMat3x3 mMain;
        ZVector3 vMain;
        pMain->GetMatPos(mMain, vMain);

        ZMat3x3 mTransposed;
        tmat(mTransposed, mMain);
        ZMat3x3 mResult = mTransposed;
        mmmul(mResult, mMat);

        ZVector3 vResult = vPos - vMain;
        TransformRootVector(vResult, mResult);
        GetItemRootTM(mResult, vResult);
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
        ZGEOM* pMain = GetMain();
        if (pMain == nullptr)
        {
            mreset(mat);
            reinterpret_cast<ZVector3*>(pos)->Reset();
            return;
        }

        auto& mOutput = *reinterpret_cast<ZMat3x3*>(mat);
        auto& vOutput = *reinterpret_cast<ZVector3*>(pos);
        pMain->GetMatPos(mOutput, vOutput);

        if (lBoneId == 31 || lBoneId == 32)
        {
            if (lBoneId == 32)
            {
                if (const ZLNKOBJ* pLinkObject = geom_cast<ZLNKOBJ>(pMain))
                {
                    const int32_t lBoneNr = pLinkObject->GetBoneNrFromName("L Hand Attacher");
                    if (lBoneNr != 0)
                    {
                        ZMat3x3 mBone;
                        ZVector3 vBone;
                        if (pLinkObject->GetBoneModifier()->GetIKBoneMatPos(
                                mBone, vBone, static_cast<uint8_t>(lBoneNr), pLinkObject, nullptr))
                        {
                            ZMat3x3 mFrom;
                            mmmul(mFrom, mBone, mOutput);
                            vmmul(vBone, mOutput);
                            vBone += vOutput;
                            mOutput = mFrom;
                            vOutput = vBone;
                        }
                    }
                }
            }

            ZMat3x3 mAdjust;
            createmat(mAdjust, ZVector3(0.0f, 0.0f, 1.0f), ZVector3(0.0f, 1.0f, 0.0f));
            ZMat3x3 mTransposed;
            tmat(mTransposed, mOutput);
            mmmul(mOutput, mTransposed, mAdjust);
            TransformRootVector(vOutput, mOutput);
            vOutput.x = -vOutput.x;
            vOutput.y = -vOutput.y;
            vOutput.z = -vOutput.z;
        }
        else
        {
            ZMat3x3 mAdjust;
            createmat(mAdjust, ZVector3(0.0f, 1.0f, 0.0f), ZVector3(1.0f, 0.0f, 0.0f));
            ZMat3x3 mTransposed;
            tmat(mTransposed, mOutput);
            mmmul(mOutput, mTransposed, mAdjust);
            TransformRootVector(vOutput, mOutput);
            vOutput.x = -vOutput.x;
            vOutput.y = -vOutput.y;
            vOutput.z = -vOutput.z;
        }
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

        if (m_pStateRemove != nullptr)
        {
            RefRun run;
            m_pStateRemove->RunInitNxtRef(&run);
            for (uint32_t* entry = m_pStateRemove->RunNxtRefPtr(&run); entry != nullptr;
                 entry = m_pStateRemove->RunNxtRefPtr(&run))
            {
                ZGEOM* geom = ZGEOM::RefToPtr(*entry);
                if (geom != nullptr)
                    geom->Delete();
            }
            m_pStateRemove->Clear();
        }

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
        ZASSERT(pTemplate != nullptr);
        if (pTemplate == nullptr)
            return;

        ZASSERT((pTemplate->GetObjectId() & ZItemTemplate::m_Mask) == ZItemTemplate::m_Id);
    }

    // vtbl slot 159  PC 0x5113E0
    void ZItem::SetItemOwner(uint32_t rOwner, ZGROUP* pGroup, bool b3, bool b4)
    {
        const uint32_t oldOwner = m_rItemOwner;
        ZGEOM* oldOwnerGeom = ZGEOM::RefToPtr(oldOwner);
        ZGEOM* newOwnerGeom = ZGEOM::RefToPtr(rOwner);
        uint32_t newOwner = rOwner;

        if (rOwner != 0 && newOwnerGeom == nullptr)
            newOwner = 0;

        if (oldOwner == newOwner)
            return;

        m_rItemOwner = newOwner;
        if (g_pSysInterface == nullptr || g_pSysInterface->m_pEngineData == nullptr ||
            !g_pSysInterface->m_pEngineData->m_LoadingGame)
        {
            if (newOwner != 0)
            {
                g_pGameData->RemoveItemOnGround(this);
                if (m_bVisibleToNPCs)
                    ZCheckVisible::m_pCheckVisible->RemoveSeeableItem(this);
            }
            else
            {
                g_pGameData->AddItemOnGround(this);
                if (b4 && m_bVisibleToNPCs)
                    ZCheckVisible::m_pCheckVisible->UpdateSeeableItem(this);
            }
        }

        ZGROUP* parent = pGroup != nullptr ? pGroup : (m_baseGeom != nullptr ? m_baseGeom->ParentGroup() : nullptr);
        if (parent != nullptr && m_baseGeom != nullptr)
        {
            if ((parent->m_baseGeom->m_lControl & 0x40040000u) != 0)
            {
                m_baseGeom->SetControl(0, 278528);
                if (newOwnerGeom == nullptr || !geom_cast<ZLNKOBJ>(newOwnerGeom))
                    parent->AttachGeom(this, true);
            }
            else
            {
                parent->AttachGeom(this, true);
                m_baseGeom->SetControl(278528, 0);
            }
        }

        if (b3 && oldOwnerGeom != nullptr)
            oldOwnerGeom->SendCommand(m_msgSetItem, nullptr, this);
        if (b3 && newOwnerGeom != nullptr)
            newOwnerGeom->SendCommand(m_msgSetItem, nullptr, this);

        EnablePickup(newOwner == 0);
    }

    // vtbl slot 160  PC 0x50F040
    ZGEOM* ZItem::GetItemOwner() const
    {
        if (m_rItemOwner == 0)
            return nullptr;
        return ZGEOM::RefToPtr(m_rItemOwner);
    }

    // vtbl slot 161  PC 0x50EF60
    ZAction* ZItem::GetAction(uint32_t lIndex)
    {
        ZAction* action = reinterpret_cast<ZAction*>(FindEvent("Action"));
        if (action == nullptr)
            return nullptr;
        return action->FindAction(nullptr, nullptr, static_cast<EActionType>(lIndex), 0);
    }

    // vtbl slot 162  PC 0x5157B0
    void* ZItem::InitPickup()
    {
        if (m_rItemTemplate == 0)
            return nullptr;

        ZAction* action = GetAction(0);
        if (action == nullptr)
            return nullptr;

        action->Hide();
        return action;
    }

    // vtbl slot 163  PC 0x50EF90
    void ZItem::EnablePickup(bool bEnable)
    {
        if (m_rItemTemplate == 0)
            return;

        ZItemTemplate* itemTemplate = GetItemTemplate();
        if (itemTemplate != nullptr && itemTemplate->GetItemHands() != IH_NONE)
        {
            ZAction* action = GetAction(0);
            ZASSERT(action != nullptr);
            if (action != nullptr)
            {
                if (bEnable)
                    action->Show();
                else
                    action->Hide();
            }
        }
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
        while (m_pGroupFirst != nullptr)
        {
            ZBaseGeom* first = m_pGroupFirst;
            first->~ZBaseGeom();
            ZUniMemory::Free(first);
        }

        if (typeId != 0)
            CreateGeom("NewObject", typeId, true);
    }

    // vtbl slot 168  PC 0x511920
    ZGEOM* ZItem::GetMarkedGeom(char const* pszName)
    {
        if (m_pStateReuse == nullptr || pszName == nullptr)
            return nullptr;

        RefRun run;
        m_pStateReuse->RunInitNxtRef(&run);
        for (uint32_t* entry = m_pStateReuse->RunNxtRefPtr(&run); entry != nullptr;
             entry = m_pStateReuse->RunNxtRefPtr(&run))
        {
            ZGEOM* geom = ZGEOM::RefToPtr(*entry);
            if (geom == nullptr)
            {
                m_pStateReuse->RunDelRef(&run);
                continue;
            }

            const char* name = geom->BaseGeom()->Name();
            if (strcasecmp(name, pszName) != 0)
                continue;

            const uint32_t ref = *entry;
            m_pStateRemove->RemoveIfExists(ref);
            m_pDeactivateStates->Remove(ref);
            m_pStateReuse->RunDelRef(&run);
            return geom;
        }

        return nullptr;
    }

    // vtbl slot 169  PC 0x50F170
    void ZItem::AddActivate(ZItemState* pItemState, float fDelay)
    {
        m_pActivateStates->AddSort(pItemState->GetRef(), fDelay, 0);
        EnableClassCall(16);
    }

    // vtbl slot 170  PC 0x50F140
    void ZItem::AddDeactivate(uint32_t rState, float fDelay)
    {
        EnableClassCall(16);
        m_pDeactivateStates->AddSort(rState, fDelay, 0);
    }

    // vtbl slot 171  PC 0x511310
    void ZItem::UpdateActivate()
    {
        const float now = static_cast<float>(g_pSysInterface->FrameTime.secs) * (1.0f / 1024.0f);
        ZGEOM* main = ZGEOM::RefToPtr(m_rMain);
        if (m_pActivateStates == nullptr)
            return;

        RefRun run;
        m_pActivateStates->RunInitNxtRef(&run);
        char eventData[4]{};
        char nextEventData[4]{};
        for (uint32_t* entry = m_pActivateStates->RunNxtRefPtr(&run); entry != nullptr;
             entry = m_pActivateStates->RunNxtRefPtr(&run))
        {
            ZGEOM* state = ZGEOM::RefToPtr(*entry);
            if (m_pActivateStates->GetSort(entry) > now)
                break;
            if (state != nullptr)
            {
                ZEventBase* event = ZEventBase::RefToPtr(*entry);
                ZGEOM* base = event != nullptr ? event->m_pBaseGeom : nullptr;
                m_pActivateStates->RunDelRef(&run);
                if (main != nullptr && base != nullptr)
                    main->SetMatPos(ZMat3x3(), ZVector3());
                (void)eventData;
                (void)nextEventData;
            }
        }
    }

    // vtbl slot 172  PC 0x50EE20
    void ZItem::UpdateDeactivate()
    {
        const float now = static_cast<float>(g_pSysInterface->FrameTime.secs) * (1.0f / 1024.0f);
        if (m_pDeactivateStates == nullptr)
            return;

        RefRun run;
        m_pDeactivateStates->RunInitNxtRef(&run);
        char eventData[4]{};
        char nextEventData[8]{};
        for (uint32_t* entry = m_pDeactivateStates->RunNxtRefPtr(&run); entry != nullptr;
             entry = m_pDeactivateStates->RunNxtRefPtr(&run))
        {
            if (m_pDeactivateStates->GetSort(entry) > now)
                break;
            ZGEOM* geom = ZGEOM::RefToPtr(*entry);
            if (geom != nullptr)
            {
                m_pDeactivateStates->RunDelRef(&run);
                m_pStateRemove->Add(*entry);
                m_pStateReuse->Add(*entry);
                geom->Delete();
            }
            else
            {
                m_pDeactivateStates->RunDelRef(&run);
            }
            (void)eventData;
            (void)nextEventData;
        }
    }

#   pragma region " --- Serialization and lifecycle --- "
    namespace
    {
        void LoadStateTable(ISerializerStream& stream, const char* name, REFTAB& table)
        {
            int32_t count = 0;
            stream.Exchange(name, count);
            for (int32_t i = 0; i < count; ++i)
            {
                uint32_t ref = 0;
                stream.Exchange("ref", ref);
                table.Add(ref);
            }
        }

        void SaveStateTable(ISerializerStream& stream, const char* name, REFTAB& table)
        {
            int32_t count = table.Count();
            stream.Exchange(name, count);
            RefRun run;
            table.RunInitNxtRef(&run);
            for (uint32_t ref = table.RunNxtRef(&run); run._RunPtr != nullptr; ref = table.RunNxtRef(&run))
                stream.Exchange("ref", ref);
        }
    }

    // ZSerializable override, PC 0x513CF0. Sorted tables serialize refs only, not their sort keys.
    void ZItem::PostSave(ISerializerStream& stream)
    {
        if (!stream.TestStreamFilter(2))
            return;

        SaveStateTable(stream, "m_pStateRemoveCount", *m_pStateRemove);
        SaveStateTable(stream, "m_pStateReuseCount", *m_pStateReuse);
        SaveStateTable(stream, "m_pDeactivateStatesCount", *m_pDeactivateStates);
        SaveStateTable(stream, "m_pActivateStatesCount", *m_pActivateStates);
    }

    // ZSerializable override, PC 0x513A90. Tables are populated, not cleared or reallocated.
    bool ZItem::PostLoad(ISerializerStream& stream)
    {
        if (stream.TestStreamFilter(2))
        {
            m_NewItem = false;
            LoadStateTable(stream, "m_pStateRemoveCount", *m_pStateRemove);
            LoadStateTable(stream, "m_pStateReuseCount", *m_pStateReuse);
            LoadStateTable(stream, "m_pDeactivateStatesCount", *m_pDeactivateStates);
            LoadStateTable(stream, "m_pActivateStatesCount", *m_pActivateStates);
        }

        return true;
    }

    // PC 0x510EF0 (slot 17); the +0x240 call is the template's CalcCenSizeRecur.
    void ZItem::CalcCenSize()
    {
        ZItemTemplate* pItemTemplate = GetItemTemplate();
        if (pItemTemplate != nullptr)
        {
            pItemTemplate->CalcCenSizeRecur();
            SetCen(pItemTemplate->Cen());
            SetSize(pItemTemplate->Size());
        }
    }

    // PC 0x50EDF0 (slot 83).
    void ZItem::ClassInit2()
    {
        ZGROUP::ClassInit2();
        InitPickup();
        EnableClassCall(16);
        CreateExData();
        m_pExData->_lControl |= ZCEXWANTCAMERAMSG;
    }

    // PC 0x4257B0 -> 0x4715C0 (slot 85); PS2 confirms the base forwarding call.
    void ZItem::PostClassInit2()
    {
        ZGROUP::PostClassInit2();
    }

    // PC 0x50EEE0 (slot 87).
    void ZItem::ClassFrameUpdate()
    {
        if (m_pDeactivateStates->Count() == 0 && m_pActivateStates->Count() == 0)
            DisableClassCall(16);

        if (m_bInMotion)
        {
            const double now = static_cast<double>(g_pSysInterface->FrameTime.secs) / 1024.0;
            if (now - m_fLastUpdatedPosition > 0.5)
            {
                ZCheckVisible::m_pCheckVisible->UpdateSeeableItem(this);
                m_fLastUpdatedPosition = static_cast<float>(now);
            }
        }

        UpdateDeactivate();
        UpdateActivate();
    }

    // PC 0x43E3B0 (slot 51): the shared four-argument no-op.
    void ZItem::CorrectOwnerDrawMatrix(ZMat3x3&, ZVector3&, ZBaseGeom*, uint32_t)
    {
    }
#   pragma endregion

#   pragma region " --- RTTI --- "
    namespace cProperties
    {
        static ZEnumEntry ItemStateEntries[] = {
            {nullptr, eIS_NORMAL, "IS_NORMAL"},
            {&ItemStateEntries[0], IS_HIDE, "IS_HIDE"},
            {&ItemStateEntries[1], IS_SHOW, "IS_SHOW"},
            {&ItemStateEntries[2], IS_ACTIVATE, "IS_ACTIVATE"},
            {&ItemStateEntries[3], IS_ACTIVATE2, "IS_ACTIVATE2"},
            {&ItemStateEntries[4], IS_EXTRA1, "IS_EXTRA1"},
            {&ItemStateEntries[5], IS_EXTRA2, "IS_EXTRA2"},
            {&ItemStateEntries[6], IS_LASTITEM, "IS_LASTITEM"},
            {&ItemStateEntries[7], IS_FORCE32, "IS_FORCE32"}};
        static ZEnumInfo ItemStateInfo{&ItemStateEntries[8], "ITEMSTATE", sizeof(ITEMSTATE)};

        // PC chain 0x80C374..0x80C418; Info.First points at 0x80C400. Declared tail-first, so the
        // head of the list (m_lCurrentState) is passed to DECLARE_GEOM_CLASS_IMPL.
        static RTP::ZDataProperty<uchar> InMotion{
            .m_Node = {.m_Next = nullptr, .m_Name = "m_bInMotion", .m_Filter = 2},
            .m_VirtualTable = &RTP::VirtualTables::Data_uchar,
            .m_Offset = reinterpret_cast<uchar*>(CLASS_PROPERTY(ZItem, m_bInMotion))};

        static RTP::ZDataProperty<float> LastPosition{
            .m_Node = {.m_Next = InMotion, .m_Name = "m_fLastUpdatedPosition", .m_Filter = 2},
            .m_VirtualTable = &RTP::VirtualTables::Data_float,
            .m_Offset = CLASS_PROPERTY(ZItem, m_fLastUpdatedPosition)};

        static RTP::ZDataProperty<uint> Vision{
            .m_Node = {.m_Next = LastPosition, .m_Name = "m_iVisionID", .m_Filter = 2},
            .m_VirtualTable = &RTP::VirtualTables::Data_uint,
            .m_Offset = CLASS_PROPERTY(ZItem, m_iVisionID)};

        static RTP::ZDataProperty<ZGEOMREF> Main{
            .m_Node = {.m_Next = Vision, .m_Name = "m_rMain", .m_Filter = 2},
            .m_VirtualTable = &RTP::VirtualTables::Data_ZGEOMREF,
            .m_Offset = reinterpret_cast<ZGEOMREF*>(CLASS_PROPERTY(ZItem, m_rMain))};

        static RTP::ZDataProperty<ZGEOMREF> Owner{
            .m_Node = {.m_Next = Main, .m_Name = "m_rItemOwner", .m_Filter = 2},
            .m_VirtualTable = &RTP::VirtualTables::Data_ZGEOMREF,
            .m_Offset = reinterpret_cast<ZGEOMREF*>(CLASS_PROPERTY(ZItem, m_rItemOwner))};

        static RTP::ZDataProperty<bool> Visible{
            .m_Node = {.m_Next = Owner, .m_Name = "m_bVisibleToNPCs", .m_Filter = 3},
            .m_VirtualTable = &RTP::VirtualTables::Data_bool,
            .m_Offset = CLASS_PROPERTY(ZItem, m_bVisibleToNPCs)};

        static RTP::ZDataProperty<ZGEOMREF> ItemTemplate{
            .m_Node = {.m_Next = Visible, .m_Name = "m_rItemTemplate", .m_Filter = 3},
            .m_VirtualTable = &RTP::VirtualTables::Data_ZGEOMREF,
            .m_Offset = reinterpret_cast<ZGEOMREF*>(CLASS_PROPERTY(ZItem, m_rItemTemplate))};

        static RTP::ZEnumProperty CurrentState{
            .m_Node = {.m_Next = ItemTemplate, .m_Name = "m_lCurrentState", .m_Filter = 2},
            .m_VirtualTable = &RTP::VirtualTables::Enum,
            .m_Offset = CLASS_PROPERTY(ZItem, m_lCurrentState),
            .m_Info = &ItemStateInfo};
    }

    DECLARE_GEOM_CLASS_IMPL(
        ZItem,           // ClassName
        ZGROUP,          // BaseClass
        0x0099C118,      // OldClassInfoAddr (dword_99C118)
        "ZItem",         // FactoryName
        0x0,             // FactoryNameAddr (documentation only; not bound by the macro)
        cProperties::CurrentState, // FirstProperty (PC 0x80C400)
        0x0080C418,      // PropertiesAddr (ZItem::Info)
        0x0099BF50,      // IdAddr (ZItem::m_Id)
        0x0099BF54       // MaskAddr (ZItem::m_Mask)
    );
#   pragma endregion
}
