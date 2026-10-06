
#include <Glacier/ZAction.h>
#include <Glacier/ZActionController.h>
#include <Glacier/EventBase/ZBaseConRout.h>
#include <Glacier/Data/ZEngineDataBase.h>
#include <Glacier/Data/ZGameData.h>
#include <Glacier/EventBase/ZEventBuffer.h>
#include <Glacier/GameBase/ZPlayer.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/IK/ZCTRLIKLNKOBJ.h>
#include <Glacier/ResourceCollection.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/IK/ZIKLNKOBJ.h>
#include <Glacier/ZSTL/StringUtils.h>
#include <Glacier/ZSTL/REFTAB32.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    namespace
    {
        bool MatchAction(ZAction* pAction, const char* pszName, EActionType eType, ZREF rReceiver)
        {
            if (pszName)
            {
                const char* pszActionName = pAction->m_szActionName.c_str();
                if (!pszActionName || strcasecmp(pszName, pszActionName) != 0)
                    return false;
            }
            if (eType != static_cast<EActionType>(-1) && pAction->m_eType != eType)
                return false;
            if (rReceiver != 0 && pAction->m_rReceiver != rReceiver)
                return false;
            return true;
        }
    }

#   pragma region " --- Static vars ---"
    STATIC_CLASS_VAR_IMPL(ZAction, ZMessageResolver, s_msgCharacterEnterRange, 0x0099CDAC, ZMessageResolver{ "CharacterEnterRange" });
    STATIC_CLASS_VAR_IMPL(ZAction, ZMessageResolver, s_msgCharacterLeaveRange, 0x0099CDB8, ZMessageResolver{ "CharacterLeaveRange" });
    STATIC_CLASS_VAR_IMPL(ZAction, ZMessageResolver, s_msgRequestGreying, 0x0099CDE8, ZMessageResolver{ "MSG_REQUESTGRAYING" });
    STATIC_CLASS_VAR_IMPL(ZAction, ZMessageResolver, s_msgCanOperateObject, 0x0099CDF4, ZMessageResolver{ "CanOperateObject" });
    STATIC_CLASS_VAR_IMPL(ZAction, ZMessageResolver, s_msgRemove, 0x0099CE00, ZMessageResolver{ "ActionRemove" });
    STATIC_CLASS_VAR_IMPL(ZAction, ZMessageResolver, s_msgEnableAction, 0x0099CE0C, ZMessageResolver{ "EnableAction" });
    STATIC_CLASS_VAR_IMPL(ZAction, ZMessageResolver, s_msgDisableAction, 0x0099CE18, ZMessageResolver{ "DisableAction" });
    STATIC_CLASS_VAR_IMPL(ZAction, ZMessageResolver, s_msgGetNearObjects, 0x0099CE24, ZMessageResolver{ "GetNearObjects" });
    STATIC_CLASS_VAR_IMPL(ZAction, ZMessageResolver, s_msgRemoveNamedAction, 0x0099CE30, ZMessageResolver{ "RemoveNamedAction" });
#   pragma endregion

    ZAction::ZAction()
    {
        m_eType = static_cast<EActionType>(1); // engine's raw default value (AT_BUTTON)
        m_msgMessage = 0;
        m_rReceiver = 0;
        m_lPriority = 0;
        m_lRange = 5;
        m_bHitmanReceiver = false;
        m_lActionControl = 0;
        m_bIsMaster = false;
        m_msgActionHide = 0;
        m_msgActionShow = 0;
        m_bIsInitialized = false;
        m_bIsItem = false;
        m_bColi2Enabled = false;
        m_lChanged = 0;
        m_pActiveActionArray = nullptr;
        UserData = 0;
    }

    ZAction::~ZAction()
    {
        ReleaseMem();
        // PC 0x52CE20: the 'deleting' destructor variant additionally calls
        // ZEventBuffer::Instance().FreeEventRam(this); event memory is released
        // through ZEventBase::ChangeStatus/STATUS_End in this project instead.
    }

    bool ZAction::InRange(ZGEOM* pTarget)
    {
        if (m_lActionControl & 5u)
            return false;

        if (m_eType == AT_ALWAYSINRANGE)
            return true;

        // PC 0x52B62A: ZPlayer-derived receivers get the geom-level CanOperateObject
        // virtual called directly with (this, null-mat-ref, null-pos-ref, false).
        // The callee must not read the reference arguments when that flag is false
        // (ZPlayer -> ZLNKWHANDS -> ZCTRLIKLNKOBJ; the engine override is
        // ZHitman3::CanOperateObject at PC 0x5FA4A0).
        if (pTarget->IsDerivedFrom<ZPlayer>())
        {
            ZMat3x3* pNullMat = nullptr;
            ZVector3* pNullPos = nullptr;
            return static_cast<ZCTRLIKLNKOBJ*>(pTarget)->CanOperateObject(this, *pNullMat, *pNullPos, false);
        }

        void* pData = this;
        const ZMSGID msg = g_pEngineData->RegisterZMsg("CanOperateObject", 0, __FILE__, __LINE__);
        pTarget->SendCommand(msg, &pData, nullptr);
        return reinterpret_cast<uintptr_t>(pData) == 1u;
    }

    ZAction* ZAction::FindAction(const char* pszName, const char* /*pszOption*/, EActionType eType, ZREF rReceiver)
    {
        if (!m_bIsInitialized)
        {
            ZGEOM* pGeom = GetGeom();
            if (reinterpret_cast<uintptr_t>(pGeom->m_pExData) == static_cast<uintptr_t>(-2))
                return nullptr;

            ZGeomEventListBuffers::ValueRun sRun {};
            pGeom->m_pExData->_Events.InitValueRun(sRun);

            auto value = pGeom->m_pExData->_Events.GetValueFromValueRun(sRun);
            while (value != 1)
            {
                auto* pEvent = ZEventBase::RefToPtr(value);
                if (pEvent && pEvent->EventName() && !striwcmp(pEvent->EventName(), "Action"))
                {
                    auto* pAction = static_cast<ZAction*>(pEvent);
                    if (MatchAction(pAction, pszName, eType, rReceiver))
                        return pAction;
                }

                pGeom->m_pExData->_Events.NextValueRun(sRun);
                value = pGeom->m_pExData->_Events.GetValueFromValueRun(sRun);
            }
            return nullptr;
        }

        // Binary convention: the array pointer stands for the ZStackArray header
        // (count at +0, entries at +4).
        ActionArray* pActions = reinterpret_cast<ActionArray*>(GetActionArray());
        for (uint32_t i = 0; i < pActions->Count(); ++i)
        {
            ZAction* pAction = *pActions->Get(i);
            if (MatchAction(pAction, pszName, eType, rReceiver))
                return pAction;
        }
        return nullptr;
    }

    void ZAction::Run(ZREF rTarget)
    {
        ZGEOM* pTarget = ZGEOM::RefToPtr(rTarget);
        if (!InRange(pTarget))
            return;

        switch (m_eType)
        {
        case AT_PLACEITEM:
        {
            ZGEOM* pGeom = GetGeom();
            const ZMSGID msg = g_pEngineData->RegisterZMsg("PlaceItem2", 0, __FILE__, __LINE__);
            // PS2 passes 'this' as the command data; the PC call reads an uninitialized value here.
            pGeom->SendCommand(pTarget, msg, this);

            if (m_rReceiver && m_msgMessage)
                pGeom->SendCommand(m_rReceiver, m_msgMessage, reinterpret_cast<void*>(static_cast<uintptr_t>(rTarget)));
            break;
        }
        case AT_GENERIC:
            if (m_rReceiver && m_msgMessage)
                GetGeom()->SendCommand(m_rReceiver, m_msgMessage, reinterpret_cast<void*>(static_cast<uintptr_t>(rTarget)));
            break;
        default:
            if (pTarget->IsDerivedFrom<ZCTRLIKLNKOBJ>())
                static_cast<ZCTRLIKLNKOBJ*>(pTarget)->OperateObject(this);
            break;
        }
    }

    void ZAction::RunMultiple(ZREF rRef)
    {
        ActionArray* pActions = reinterpret_cast<ActionArray*>(GetActionArray());

        ZAction* pMatched[32];
        uint32_t lMatched = 0;

        const bool bMatchAll = m_szActionName.m_StringObject == nullptr
            || reinterpret_cast<uintptr_t>(m_szActionName.m_StringObject) == static_cast<uintptr_t>(-12);

        for (uint32_t i = 0; i < pActions->Count(); ++i)
        {
            ZAction* pAction = *pActions->Get(i);
            if (bMatchAll)
            {
                ZASSERT(lMatched < 32);
                pMatched[lMatched++] = pAction;
                continue;
            }

            const char* pszOther = pAction->m_szActionName.c_str();
            if (pszOther && strcasecmp(m_szActionName.c_str(), pszOther) == 0)
            {
                ZASSERT(lMatched < 32);
                pMatched[lMatched++] = pAction;
            }
        }

        for (uint32_t i = 0; i < lMatched; ++i)
            pMatched[i]->Run(rRef);
    }

    void ZAction::RunFinished(ZGEOM*) {}

    void ZAction::ChangeNames(const char* pszNames)
    {
        m_szActionName.Cleanup();
        m_szActionName = pszNames;

        if (m_bIsInitialized)
            SendActionChange(reinterpret_cast<void*>(static_cast<uintptr_t>(1)));
    }

    void ZAction::SetType(EActionType eType)
    {
        m_eType = eType;

        if (m_bIsInitialized)
            SendActionChange(reinterpret_cast<void*>(static_cast<uintptr_t>(1)));
    }

    void ZAction::SetMessage(ZMSGID msgMessage)
    {
        m_msgMessage = msgMessage;
    }

    void ZAction::SetPriority(unsigned int lPriority)
    {
        m_lPriority = static_cast<int32_t>(lPriority);

        if (m_bIsInitialized)
            SendActionChange(reinterpret_cast<void*>(static_cast<uintptr_t>(1)));
    }

    void ZAction::SafeDelete()
    {
        // PS2 0x2B9E24 / PC 0x52AFA0: defer deletion to ZActionController::FrameUpdate
        // by queueing this action's ref, so it is ended outside the action iteration.
        if (ZActionController* pController = ZActionController::GetCurrentController(false))
        {
            if (!ZActionController::m_pDeleteActions)
                ZActionController::m_pDeleteActions = ZUniMemory::New<REFTAB32>();
            ZActionController::m_pDeleteActions->Add(GetRef());
        }
    }

    void ZAction::Initialize(
        const char* szActionName,
        const char* szOptionName,
        EActionType eType,
        ZMSGID msgMessage,
        ZREF rReceiver,
        int lPriority,
        int lRange,
        ZREF rItemTemplate)
    {
        SetNames(szActionName, szOptionName);
        m_eType = eType;
        m_msgMessage = msgMessage;
        m_rReceiver = rReceiver;
        SetPriority(static_cast<unsigned int>(lPriority));
        m_lRange = lRange;
        UserData = rItemTemplate;
        m_msgActionHide = g_pEngineData->RegisterZMsg("ActionHide", 0, __FILE__, __LINE__);
        m_msgActionShow = g_pEngineData->RegisterZMsg("ActionShow", 0, __FILE__, __LINE__);
    }

    void ZAction::ActionFrameUpdate(ZGEOM* pGeom)
    {
        if (!m_bIsItem)
        {
            UpdateObjectsInRange(pGeom);
            return;
        }

        if (!(GetGeom()->BaseGeom()->Control() & 0x400u))
            UpdateObjectsInRange(pGeom);
    }

    ZAction** ZAction::GetActionArray()
    {
        if (m_pActiveActionArray)
            return reinterpret_cast<ZAction**>(m_pActiveActionArray);

        ZGEOM* pGeom = GetGeom();
        const ZMSGID msg = g_pEngineData->RegisterZMsg("Action_GetActionList", 0, __FILE__, __LINE__);
        pGeom->SendCommand(msg, &m_pActiveActionArray, pGeom);

        if (!m_pActiveActionArray)
        {
            m_pActiveActionArray = &m_ActionArray;
            m_bIsInitialized = true;
        }
        return reinterpret_cast<ZAction**>(m_pActiveActionArray);
    }

    void ZAction::Show()
    {
        m_lActionControl &= ~1u;

        if (m_bIsMaster)
            ZActionController::GetCurrentController(true)->UpdateSingleObject(this);
    }

    void ZAction::Hide()
    {
        m_lActionControl |= 1u;
    }

    ZAction* ZAction::AddAction(
        ZGEOM* pGeom,
        const char* psLocalizedActionName,
        const char* psActionName,
        EActionType actionType,
        Glacier::ZMSGID commandId,
        Glacier::ZREF entityRef,
        int unk0,
        int radius)
    {
        auto* pEvent = pGeom->AddEvent("ZGEOM_Action");
        if (!pEvent)
            return nullptr;

        auto* pAction = static_cast<ZAction*>(pEvent);

        // PS2 0x2B9584 / PC resolve the localized action text through the engine's
        // locale resource collection (g_pEngineData->m_pLocaleResources; vtbl slot +4
        // is ResourceCollection::GetResourceText(base, name)) with an empty base
        // string before initializing. An unregistered key falls back to the raw name.
        const char* pszLocalized =
            g_pEngineData->m_pLocaleResources->GetResourceText("", psLocalizedActionName);

        pAction->Initialize(
            pszLocalized,
            psActionName,
            actionType,
            commandId,
            entityRef,
            unk0,
            radius,
            0);

        return pAction;
    }

    void ZAction::SetNames(const char* szActionName, const char* szOriginalName)
    {
        m_szActionName.Cleanup();
        m_szActionName = szActionName;

        if (szOriginalName)
            m_szOriginalString = szOriginalName;
    }

    void ZAction::ReleaseMem()
    {
        m_szActionName.Cleanup();
        m_szOriginalString.Cleanup();
    }

    void ZAction::SendActionChange(void* pData)
    {
        ZGEOM* pGeom = GetGeom();
        ZASSERT(pGeom);

        const ZMSGID msg = g_pEngineData->RegisterZMsg("ACTIONCHANGE", 0, __FILE__, __LINE__);
        ZASSERT(msg);
        pGeom->SendCommand(msg, pData, pGeom);
    }

    void ZAction::UpdateObjectsInRange(ZGEOM* pGeom)
    {
        if (!pGeom)
            return;

        ZASSERT(m_bIsInitialized == (m_pActiveActionArray == &m_ActionArray));

        bool bAnyInRange = false;

        for (uint32_t i = 0; i < m_ActionArray.Count(); ++i)
        {
            ZAction* pAction = *m_ActionArray.Get(i);
            if (!pAction)
                continue;

            if (!pAction->InRange(pGeom))
            {
                if (!(pAction->m_lActionControl & 2u) && pAction->m_bIsInitialized)
                    pAction->SendActionChange(reinterpret_cast<void*>(static_cast<uintptr_t>(2)));
                pAction->m_lActionControl |= 2u;
            }
            else
            {
                bAnyInRange = true;
                if ((pAction->m_lActionControl & 2u) && pAction->m_bIsInitialized)
                    pAction->SendActionChange(reinterpret_cast<void*>(static_cast<uintptr_t>(2)));
                pAction->m_lActionControl &= ~2u;
            }
        }

        // PC 0x52C890 calls DisableActions when at least one action is in range,
        // EnableActions otherwise (PS2 keeps the opposite Enable/Disable naming).
        if (bAnyInRange)
            DisableActions(pGeom);
        else
            EnableActions(pGeom);
    }

    void ZAction::EnableActions(ZGEOM* pTarget)
    {
        if (!m_bIsInitialized)
            return;

        uint32_t lRef = pTarget->GetRef();
        const int32_t lIndex = m_NearObjects.Find(&lRef);
        if (lIndex < 0)
            return;
        m_NearObjects.Remove(static_cast<uint32_t>(lIndex));

        ZASSERT(m_bIsInitialized == (m_pActiveActionArray == &m_ActionArray));

        auto* pPlayer = g_pGameData ? g_pGameData->GetPlayer(0) : nullptr;
        if (pPlayer && static_cast<void*>(pTarget) == static_cast<void*>(pPlayer))
        {
            for (uint32_t i = 0; i < m_ActionArray.Count(); ++i)
                (*m_ActionArray.Get(i))->m_lActionControl |= 2u;
        }

        ZGEOM* pGeom = GetGeom();
        const ZMSGID msg = g_pEngineData->RegisterZMsg("MSG_ENTERITEMRANGE", 0, __FILE__, __LINE__);
        pGeom->SendCommand(pTarget, msg, reinterpret_cast<void*>(static_cast<uintptr_t>(pGeom->GetRef())));
    }

    void ZAction::DisableActions(ZGEOM* pTarget)
    {
        if (!m_bIsMaster)
            return;

        if (!pTarget->IsDerivedFrom<ZIKLNKOBJ>())
            return;

        uint32_t lRef = pTarget->GetRef();
        if (m_NearObjects.Find(&lRef) < 0 && m_NearObjects.Count() < m_NearObjects.TotalNrEntries())
        {
            m_NearObjects.Push(lRef);

            ZGEOM* pGeom = GetGeom();
            const ZMSGID msg = g_pEngineData->RegisterZMsg("MSG_LEAVEITEMRANGE", 0, __FILE__, __LINE__);
            pGeom->SendCommand(pTarget, msg, reinterpret_cast<void*>(static_cast<uintptr_t>(pGeom->GetRef())));
        }
    }
}
