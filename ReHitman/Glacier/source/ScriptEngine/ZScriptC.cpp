#include <Glacier/ScriptEngine/ZScriptC.h>
#include <Glacier/EventBase/ZScheduledScript.h>
#include <Glacier/ScriptEngine/AsyncCall_Struct.h>
#include <Glacier/ScriptEngine/FUNCTIONCONTROLLER.h>
#include <Glacier/ScriptEngine/Globals.h>
#include <Glacier/ScriptEngine/LocalVarEntry.h>
#include <Glacier/ScriptEngine/MessageCue.h>
#include <Glacier/ScriptEngine/ScriptEngine.h>
#include <Glacier/ScriptEngine/ScriptFlags.h>
#include <Glacier/ScriptEngine/ScriptState.h>
#include <Glacier/ScriptEngine/ScriptStateInfo.h>
#include <Glacier/ScriptEngine/SCRIPTCREATOR.h>
#include <Glacier/ScriptEngine/SpecialScriptReturnType.h>
#include <Glacier/ScriptEngine/STATECONTROLLER.h>
#include <Glacier/System/ZSysInterface.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/ZEngineDataBase.h>
#include <Glacier/ZMessageResolver.h>
#include <Glacier/ZSTL/ZOffsetAlloc.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZUniAssert.h>
#include <malloc.h>
#include <cstring>
#include <cstdlib>


namespace Glacier
{
    namespace
    {
        float ForkStateFree(ScriptState* pState)
        {
            LocalVarEntry* pFreeVariables = pState->m_pVariables;
            LocalVarEntry* pRunVariables = pFreeVariables->m_pPrevVariables;

            ScriptEngine::Free(pFreeVariables->m_pNextVariables);
            ScriptEngine::Free(pFreeVariables);

            pRunVariables->m_pNextVariables = nullptr;
            pState->m_pVariables = pRunVariables;

            return -3.0f;
        }

        const FUNCTIONCONTROLLER ForkStateFree_FUNCTIONCONTROLLER
        {
            reinterpret_cast<EntryPoint_t>(ForkStateFree),
            0,
            0,
            nullptr,
            nullptr
        };

        ZMessageResolver m_MSG_CAM_ENTERCAMERA("CAM_ENTERCAMERA");
        uint32_t lMessageAllocatorCount = 0;

        // Dump from PC build
        static constexpr uint16_t ZMSG_Sizes[786] =
        {
            0x8, 0x8, 0x0, 0x4, 0x18, 0x0, 0x0, 0x0, 0xC, 0xC, 0x4, 0x4, 0x4, 0x18, 0x0, 0x0,
            0x4, 0x4, 0x0, 0x0, 0x10, 0x10, 0x10, 0x4, 0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4,
            0x4, 0x0, 0x4, 0x8, 0x4, 0x0, 0x0, 0x4, 0x4, 0x8, 0x4, 0x0, 0x0, 0x0, 0x0, 0x4,
            0x0, 0x8, 0x8, 0x0, 0x0, 0x0, 0x0, 0x4, 0x4, 0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x4, 0x0, 0x0, 0x4, 0x4, 0x4, 0x4, 0x4, 0x8, 0x0, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x44, 0x4, 0x8, 0x8, 0x4, 0x4, 0x8, 0x4, 0x8,
            0xC, 0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4, 0x4, 0x0, 0x0, 0x4,
            0x4, 0x4, 0x8, 0x0, 0x0, 0x0, 0x8, 0x0, 0x0, 0x4, 0x4, 0x0, 0x0, 0x0, 0x1, 0x0,
            0x0, 0x4, 0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4, 0x8, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
            0x4, 0x4, 0x0, 0x0, 0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x4, 0xC, 0x8, 0xC, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4, 0x1, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4,
            0x8, 0x0, 0x4, 0x0, 0x10, 0x4, 0x0, 0x0, 0x0, 0x8, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x1,
            0x4, 0x4, 0x0, 0x0, 0x0, 0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4, 0x4, 0x4, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x4, 0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x8, 0x0, 0x28,
            0x2C, 0x38, 0xC, 0x24, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4,
            0x4, 0x0, 0x0, 0x0, 0x10, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4, 0x0, 0x0, 0x8, 0x0, 0x0, 0x20, 0x4, 0x8,
            0xC, 0x0, 0x0, 0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x4, 0x0, 0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4, 0x4, 0x0,
            0x0, 0x0, 0x0, 0x4, 0x1, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4, 0x0, 0x8,
            0x8, 0x10, 0x4, 0x4, 0x4, 0x4, 0x8, 0x4, 0x0, 0x4, 0x0, 0x0, 0x4, 0x8, 0x0, 0x0,
            0x8, 0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0,
            0x1, 0x8, 0x0, 0x0, 0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x8, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x1, 0x2, 0x0, 0x0, 0x1,
            0x0, 0x0, 0x0, 0x0, 0x3, 0x1, 0x0, 0x0, 0x0, 0x0, 0x1, 0x1, 0x1, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0, 0x2, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x4, 0x0,
            0x0, 0x0, 0x4, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4, 0xC, 0x0, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
            0xC, 0x0, 0x8, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4, 0x0, 0x0, 0x4, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4, 0x0, 0x8, 0x0, 0x0, 0x0, 0x4, 0x0,
            0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4, 0x0, 0x0, 0x0, 0x8,
            0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
            0x4, 0x18, 0x0, 0x0, 0x0, 0x4, 0x4, 0x4, 0x0, 0x0, 0x0, 0x1, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x4, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0, 0x2, 0x2,
            0x1, 0x3, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0, 0x8, 0x0, 0x0, 0x0, 0x0,
            0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
            0x1, 0x0, 0x1, 0x1, 0x0, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4, 0x4,
            0x4, 0x4, 0x4, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x4,
            0x8, 0x0,
        };

        int GetMessageBodySize(ZMSGID Msg)
        {
            if (Msg < 0x800u)
            {
                return -1;
            }

            const uint32_t lIndex = Msg - 0x800u;
            if (lIndex >= sizeof(ZMSG_Sizes) / sizeof(ZMSG_Sizes[0]))
            {
                return -1;
            }

            return ZMSG_Sizes[lIndex];
        }
    }

    ZScriptC::ZScriptC()
        : CBaseEvent<ZGEOM>()
    {
        m_pScriptCreator = nullptr;
        m_pStoredDataBlock = nullptr;
    }

    ZScriptC::~ZScriptC()
    {
        if (auto* pScript = const_cast<ZScheduledScript*>(GetSchedEvent()))
        {
            ZScheduledScript* pFirstScript = pScript;
            ZScheduledScript* pNextScript = nullptr;

            do
            {
                FreeThread(static_cast<ScriptStateInfo*>(pScript->GetUserData()));
                pNextScript = static_cast<ZScheduledScript*>(pScript->GetNextThread());
                pScript = pNextScript;
            }
            while (pNextScript != pFirstScript && pNextScript);
        }

        ScriptEngine::DetachSceneScripts();
        if (m_pStoredDataBlock)
        {
            ZUniMemory::Free(m_pStoredDataBlock);
            m_pStoredDataBlock = nullptr;
        }
    }

    void ZScriptC::PostSave(ISerializerStream& stream)
    {
        // TODO: Finish me
    }

    bool ZScriptC::PostLoad(ISerializerStream& stream)
    {
        // TODO: Finish me
        return false;
    }

    const RTP::ZPropertyInfo& ZScriptC::GetProperties() const
    {
        return ZScriptC::Info;
    }

    void ZScriptC::Init2()
    {
        // TODO: Finish me
    }

    void ZScriptC::PostInit()
    {
        if (s_pLoadEntries)
        {
            ZUniMemory::Free(s_pLoadEntries);
            s_pLoadEntries = nullptr;
        }
    }

    void ZScriptC::PreSaveGame()
    {
        if (!s_bSaving)
        {
            s_lObjectSaveCount = 0;
            s_bSaving = true;
        }

        s_pCurrentSaveGameObject = this;

        ZScheduledEvent* pFirstThread = m_pScheduleEvent;
        ZScheduledEvent* pThread = pFirstThread;
        if (pThread)
        {
            do
            {
                auto* pScheduledScript = static_cast<ZScheduledScript*>(pThread);
                auto* pScriptStateInfo = static_cast<ScriptStateInfo*>(pScheduledScript->GetUserData());
                if (pScriptStateInfo)
                {
                    // TODO: Finish this place after PrepareSave will be reversed
                    // PrepareSave(pScriptStateInfo->m_pRootScriptState);
                }

                pThread = pThread->GetNextThread();
            }
            while (pThread && pThread != pFirstThread);
        }

        ++s_lObjectSaveCount;
    }

    void ZScriptC::FrameUpdate()
    {
        SchedUpdate();
        DeactivateFrameUpdate();
    }

    int ZScriptC::Command(ZMSGID command, ZDATA data)
    {
        ZScheduledEvent* pFirstThread = m_pScheduleEvent;
        ZScheduledEvent* pThread = pFirstThread;

        if (pThread)
        {
            do
            {
                auto* pScheduledScript = static_cast<ZScheduledScript*>(pThread);
                auto* pScriptStateInfo = static_cast<ScriptStateInfo*>(pScheduledScript->GetUserData());
                if (pScriptStateInfo)
                {
                    ScriptState* pState = pScriptStateInfo->m_pRootScriptState;
                    g_pZSC = pScheduledScript;

                    if ((pState->m_Flags & ZSC_FLAG_SUPPRESS_MESSAGE_COMMAND) == 0)
                    {
                        if ((pState->m_Flags & ZSC_FLAG_SKIP_MESSAGE_QUEUE) != 0)
                        {
                            const uint16_t lQueuedMessageCount = pState->m_Flags >> 12;
                            if (lQueuedMessageCount != 15)
                            {
                                const int lMessageBodySize = GetMessageBodySize(command);
                                if (lMessageBodySize >= 0)
                                {
                                    bool bMessageAlreadyQueued = false;
                                    for (MessageCue* pMessageCue = pState->m_pMessageCue; pMessageCue; pMessageCue = pMessageCue->m_pNext)
                                    {
                                        if (pMessageCue->msg == command && memcmp(pMessageCue->GetData(), data, lMessageBodySize) == 0)
                                        {
                                            bMessageAlreadyQueued = true;
                                            break;
                                        }
                                    }

                                    if (!bMessageAlreadyQueued)
                                    {
                                        auto* pMessageCue = static_cast<MessageCue*>(ScriptEngine::AllocNM(lMessageBodySize + sizeof(MessageCue), __FILE__, __LINE__));
                                        if (pMessageCue)
                                        {
                                            pMessageCue->msg = command;
                                            if (lMessageBodySize && data)
                                            {
                                                memcpy(pMessageCue->GetData(), data, lMessageBodySize);
                                            }

                                            if (pState->m_pMessageCue)
                                            {
                                                MessageCue* pLastMessageCue = pState->m_pMessageCue->m_pLast;
                                                pLastMessageCue->m_pNext = pMessageCue;
                                                pMessageCue->m_pLast = nullptr;
                                                pMessageCue->m_pNext = nullptr;
                                                pState->m_pMessageCue->m_pLast = pMessageCue;
                                            }
                                            else
                                            {
                                                pState->m_pMessageCue = pMessageCue;
                                                pMessageCue->m_pLast = pMessageCue;
                                                pState->m_pMessageCue->m_pLast->m_pNext = nullptr;
                                            }

                                            pState->m_Flags = (pState->m_Flags & ~ZSC_MESSAGE_QUEUE_COUNT_MASK) |
                                                (((lQueuedMessageCount + 1u) << 12) & ZSC_MESSAGE_QUEUE_COUNT_MASK);
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            const SCRIPTCREATOR* pPreviousMessageHandler = pState->m_pMessageHandler;
                            const bool bWasHandlingMessage = (pState->m_Flags & ZSC_FLAG_HANDLING_MESSAGE) != 0;
                            pState->m_Flags |= ZSC_FLAG_HANDLING_MESSAGE;
                            pState->m_pMessageHandler = pState->m_pCreator;

                            const bool bHadAsyncCall = pState->m_pAsyncCall != nullptr;

                            if (!g_pMessageAllocator)
                            {
                                ZASSERT(lMessageAllocatorCount == 0);
                                static ZOffsetAlloc::ZLink messageAllocatorLinks[0x20];
                                static alignas(ZOffsetAlloc) uint8_t messageAllocatorBuffer[0x1000];
                                g_pMessageAllocator = new (messageAllocatorBuffer) ZOffsetAlloc(0x114, 0x1000, 0x20, messageAllocatorLinks, EAllocType::DEFAULT_MEM);
                            }

                            ++lMessageAllocatorCount;

                            if (pState->m_pStateController && pState->m_pVariables)
                            {
                                LocalVarEntry* pVariables = pState->m_pVariables;
                                const uint16_t lExitFunctionIndex = pVariables->m_lExitFunctionIndex;
                                pVariables->m_lExitFunctionIndex |= 0x8000u;

                                using ProcessMessageFunction_t = void(*)(ScriptState*, ZMSGID, const void*);
                                auto* pProcessMessage = reinterpret_cast<ProcessMessageFunction_t>(pState->m_pStateController->ProcessMessage);
                                if (pProcessMessage)
                                {
                                    pProcessMessage(pState, command, data);
                                }
                                else
                                {
                                    const SCRIPTCREATOR* pMessageHandler = pState->m_pMessageHandler;
                                    while (pMessageHandler)
                                    {
                                        pState->m_pMessageHandler = pMessageHandler->m_pParentCreator;
                                        if (pMessageHandler->ProcessMessage)
                                        {
                                            reinterpret_cast<ProcessMessageFunction_t>(pMessageHandler->ProcessMessage)(pState, command, data);
                                            break;
                                        }

                                        pMessageHandler = pMessageHandler->m_pParentCreator;
                                    }
                                }

                                if ((lExitFunctionIndex & 0x8000u) == 0)
                                {
                                    pVariables->m_lExitFunctionIndex &= ~0x8000u;
                                }
                            }

                            pState->m_pMessageHandler = pPreviousMessageHandler;
                            if (!bWasHandlingMessage)
                            {
                                pState->m_Flags &= ~ZSC_FLAG_HANDLING_MESSAGE;
                            }

                            if (!bHadAsyncCall && pState->m_pAsyncCall)
                            {
                                pState->m_pAsyncCall->m_fStoredNextRun = static_cast<float>(pThread->m_tNextRun.secs) / TIMETYPE::kTicksPerSecond;
                                pScheduledScript->Sleep(0.0f);
                            }

                            --lMessageAllocatorCount;
                        }
                    }
                }

                ZASSERT(m_MSG_CAM_ENTERCAMERA.m_MessageID != 0);
                if (command == m_MSG_CAM_ENTERCAMERA.m_MessageID && pThread->m_lPriority > 6)
                {
                    pThread->SetPriority(6u);
                }

                pThread = pThread->GetNextThread();
            }
            while (pThread && pThread != pFirstThread);
        }

        if (g_pMessageAllocator && lMessageAllocatorCount == 0)
        {
            ZASSERT(g_pMessageAllocator->GetNrFreeLinks() == 1 && g_pMessageAllocator->GetFreeTotal() == 3820);
            g_pMessageAllocator->~ZOffsetAlloc();
            g_pMessageAllocator = nullptr;
        }

        g_pZSC = nullptr;
        return 0;
    }

    void ZScriptC::SchedUpdate()
    {
        auto* pScheduledScript = static_cast<ZScheduledScript*>(m_pScheduleEvent);
        auto* pScriptStateInfo = static_cast<ScriptStateInfo*>(pScheduledScript->GetUserData());
        if (!pScriptStateInfo)
        {
            DeactivateScheduleUpdate();
            m_lRoutCases &= ~0x20;
            ZEventBase::Remove();
            return;
        }

        ScriptEngine::SetRunningThread(pScriptStateInfo->m_pRootScriptState);
        s_CurrentRootScriptCRef = m_Ref;
        g_lCurrentScriptMaxTime = static_cast<uint32_t>(g_pSysInterface->TimeStampCounter(__FILE__, __LINE__));

        ScriptState* pState = pScriptStateInfo->m_pCurrentScriptState;
        bool bAlienAsyncCall = false;

        while (true)
        {
            if (!pState || !pState->m_pVariables)
            {
                pScheduledScript->m_tNextRun = TIMETYPE(-2);
                return;
            }

            float fStoredNextRun = 0.0f;
            float fResult = 0.0f;
            uint16_t flags = 0;
            AsyncCall_Struct* pAsyncCall = nullptr;
            const FUNCTIONCONTROLLER* pFunctionController = nullptr;
            auto CheckScriptTime = [&]() -> bool
            {
                // PC yields only after the script spent 5000 timestamp ticks in this scheduler slice.
                if (g_pSysInterface->TimeStampCounter(__FILE__, __LINE__) - g_lCurrentScriptMaxTime >= 5000)
                {
                    pScheduledScript->m_tNextRun.secs = 0;
                    return (pState->m_Flags & ZSC_CONTINUE_AFTER_SLEEP_MASK) == 0;
                }

                return false;
            };

            if ((pState->m_Flags & ZSC_FLAG_ALIEN_CALL_ACTIVE) != 0)
            {
                ScriptState* pAlienState = pScriptStateInfo->m_pRootScriptState;
                while (pAlienState)
                {
                    if ((pAlienState->m_Flags & ZSC_ALIEN_ASYNC_BLOCK_MASK) == 0 && pAlienState->m_pAsyncCall)
                    {
                        pState = pAlienState;
                        bAlienAsyncCall = true;
                        pAsyncCall = pAlienState->m_pAsyncCall;
                        break;
                    }

                    pAlienState = pAlienState->m_pAlienCall;
                }
            }

            while (!pAsyncCall)
            {
                if ((pState->m_Flags & ZSC_LOCAL_ASYNC_BLOCK_MASK) == 0)
                {
                    pAsyncCall = pState->m_pAsyncCall;
                    if (pAsyncCall)
                    {
                        break;
                    }
                }

                if (!bAlienAsyncCall)
                {
                    break;
                }

                pState = pScriptStateInfo->m_pCurrentScriptState;
                bAlienAsyncCall = false;
            }

            if (pAsyncCall)
            {
                fStoredNextRun = pAsyncCall->m_fStoredNextRun;
                pFunctionController = pAsyncCall->m_pLVE->m_pFunctionController;
                pState->m_Flags |= ZSC_FLAG_ASYNC_ACTIVE;
            }
            else
            {
                if ((pState->m_Flags & ZSC_FLAG_SKIP_MESSAGE_QUEUE) == 0)
                {
                    MessageCue* pMessageCue = pState->m_pMessageCue;
                    if (pMessageCue)
                    {
                        if ((pState->m_Flags & ZSC_FLAG_SUPPRESS_MESSAGE_COMMAND) == 0)
                        {
                            // Message payload is stored inline immediately after the 0x0C-byte queue header.
                            Command(pMessageCue->msg, pMessageCue->GetData());
                        }

                        pState->m_pMessageCue = pMessageCue->m_pNext;
                        if (pState->m_pMessageCue)
                        {
                            pState->m_pMessageCue->m_pLast = pMessageCue->m_pLast;
                        }

                        ScriptEngine::Free(pMessageCue);
                        pState->m_Flags = (pState->m_Flags & ~ZSC_MESSAGE_QUEUE_COUNT_MASK) |
                            ((((pState->m_Flags >> 12) - 1u) << 12) & ZSC_MESSAGE_QUEUE_COUNT_MASK);
                        if (CheckScriptTime())
                        {
                            return;
                        }

                        continue;
                    }
                }

                if (!pState->m_pVariables->m_pFunctionController)
                {
                    ScriptEngine::Free(pState->m_pVariables);

                    uint32_t lVariableSize = pState->m_pStateController->m_pRun->m_lDataSize;
                    if (lVariableSize < sizeof(LocalVarEntry))
                    {
                        lVariableSize = sizeof(LocalVarEntry);
                    }

                    pState->m_pVariables = static_cast<LocalVarEntry*>(ScriptEngine::Alloc(lVariableSize, __FILE__, __LINE__));
                    if (!pState->m_pVariables)
                    {
                        return;
                    }

                    pState->m_pVariables->m_pFunctionController = pState->m_pStateController->m_pRun;
                    ZASSERT(pState->m_pVariables->m_pFunctionController != nullptr);
                    pState->m_pVariables->m_lFunctionIndex = 0;
                    pState->m_pVariables->m_lExitFunctionIndex = 0xFFFFu;
                }

                pFunctionController = pState->m_pVariables->m_pFunctionController;
            }

            g_pCurrentSS = pState;
            // PC calls the entrypoint as void-typed data, then immediately fstp's ST0 as the script result.
            using SchedEntryPoint_t = float(*)(ScriptState*);
            fResult = reinterpret_cast<SchedEntryPoint_t>(pFunctionController->m_pEntryPoint)(pState);
            pState->m_Flags &= ~ZSC_FLAG_CLEAR_AFTER_ENTRY;

            if (!bAlienAsyncCall && (pState->m_Flags & ZSC_FLAG_ALIEN_CALL_ACTIVE) != 0)
            {
                if (pState->m_pAlienCall)
                {
                    pState = pState->m_pAlienCall;
                }

                if (!pState->m_pVariables->m_pFunctionController)
                {
                    ScriptState* pAlienState = pState;
                    pState = pScriptStateInfo->m_pRootScriptState;

                    if (pScriptStateInfo->m_pRootScriptState->m_pAlienCall != pAlienState)
                    {
                        while (pState->m_pAlienCall != pAlienState)
                        {
                            pState = pState->m_pAlienCall;
                        }
                    }

                    if (pState == pScriptStateInfo->m_pRootScriptState)
                    {
                        pState->m_Flags &= ~ZSC_FLAG_ALIEN_CALL_ACTIVE;
                    }

                    if ((pAlienState->m_Flags & ZSC_FLAG_ALIEN_ASYNC_UNLINK) != 0)
                    {
                        pState->m_Flags &= ~ZSC_FLAG_ALIEN_ASYNC_UNLINK;
                        AsyncCall_Struct* pStateAsyncCall = pState->m_pAsyncCall;
                        pAlienState->m_pVariables->m_pPrevVariables = pStateAsyncCall->m_pLVE;
                        pStateAsyncCall->m_pLVE->m_pNextVariables = pAlienState->m_pVariables;
                        ScriptEngine::Free(pAlienState);
                    }
                    else
                    {
                        pAlienState->m_pVariables->m_pPrevVariables = pState->m_pVariables;
                        pState->m_pVariables->m_pNextVariables = pAlienState->m_pVariables;
                        ScriptEngine::Free(pAlienState);
                    }

                    pState->m_pAlienCall = nullptr;
                }

                pScriptStateInfo->m_pCurrentScriptState = pState;
            }

            if ((pState->m_Flags & ZSC_ASYNC_RESUME_CLEAR_MASK) == ZSC_ASYNC_RESUME_CLEAR_MASK)
            {
                pState->m_Flags &= ~1u;
            }

            flags = pState->m_Flags;
            if ((flags & (ZSC_FLAG_ASYNC_ACTIVE | ZSC_FLAG_ASYNC_WAITING)) ==
                (ZSC_FLAG_ASYNC_ACTIVE | ZSC_FLAG_ASYNC_WAITING))
            {
                pState->m_Flags = flags & ~ZSC_FLAG_ASYNC_ACTIVE;
                if (CheckScriptTime())
                {
                    return;
                }

                continue;
            }

            if ((flags & ZSC_FLAG_ASYNC_ACTIVE) == 0 || pState->m_pAsyncCall)
            {
                // PC converts the float result with __ftol2 before dispatching special returns -7..-2.
                const int lScriptResult = static_cast<int>(fResult);
                switch (lScriptResult)
                {
                    case -7:
                        pScheduledScript->m_tNextRun.secs = 0;
                        return;

                    case -6:
                    {
                        uint32_t lPriority = ScriptEngine::m_SpecialScriptReturnType.m_lPriority;
                        if (lPriority > 15u)
                        {
                            lPriority = 15u;
                        }

                        pScheduledScript->SetPriority(lPriority);
                        continue;
                    }

                    case -5:
                        continue;

                    case -4:
                        ScriptEngine::m_SpecialScriptReturnType.m_pForkReturnScriptState =
                            ForkState(ScriptEngine::m_SpecialScriptReturnType.m_pForkStateController);
                        continue;

                    case -3:
                        if (CheckScriptTime())
                        {
                            return;
                        }

                        continue;

                    case -2:
                        pScheduledScript->m_tNextRun = TIMETYPE(-2);
                        TerminateScript();
                        return;

                    default:
                        pScheduledScript->m_tNextRun.secs = static_cast<int>(fResult * TIMETYPE::kTicksPerSecond);
                        break;
                }
            }
            else
            {
                pState->m_Flags = flags & ~ZSC_FLAG_ASYNC_ACTIVE;

                int lNextRun = static_cast<int>(fStoredNextRun * TIMETYPE::kTicksPerSecond);
                if (lNextRun > 0)
                {
                    lNextRun -= g_pSysInterface->FrameTime.secs;
                    if (lNextRun < 0)
                    {
                        if (CheckScriptTime())
                        {
                            return;
                        }

                        continue;
                    }
                }

                if (lNextRun == 0)
                {
                    if (CheckScriptTime())
                    {
                        return;
                    }

                    continue;
                }

                pScheduledScript->m_tNextRun.secs = lNextRun;
            }

            // These flags keep the script runnable even after m_tNextRun was updated.
            if ((pState->m_Flags & ZSC_CONTINUE_AFTER_SLEEP_MASK) == 0)
            {
                return;
            }

            continue;
        }
    }

    ScriptStateInfo* ZScriptC::CreateScript(const SCRIPTCREATOR* pCreator)
    {
        const ZREF rThis = m_pBaseGeom->GetRef();
        void* pStateVariables = nullptr;
        void* pScriptVariables = nullptr;
        auto OutOfMemory = [&]() -> ScriptStateInfo*
        {
            printf(
                "Out of script memory creating script %s (script data size:%d, state data size:%d, ref:%x)\n",
                pCreator->m_pName,
                pCreator->m_lScriptVariablesSize,
                pCreator->m_lStateVariablesSize,
                rThis);

            return nullptr;
        };

        auto* pScriptStateInfo = static_cast<ScriptStateInfo*>(ScriptEngine::Alloc(sizeof(ScriptStateInfo), __FILE__, __LINE__));
        if (!pScriptStateInfo)
        {
            return OutOfMemory();
        }

        auto* pState = static_cast<ScriptState*>(ScriptEngine::Alloc(sizeof(ScriptState), __FILE__, __LINE__));
        if (!pState)
        {
            ScriptEngine::Free(pScriptStateInfo);
            return OutOfMemory();
        }

        if (pCreator->m_lStateVariablesSize)
        {
            pStateVariables = ScriptEngine::Alloc(pCreator->m_lStateVariablesSize, __FILE__, __LINE__);
            if (!pStateVariables)
            {
                ScriptEngine::Free(pState);
                ScriptEngine::Free(pScriptStateInfo);
                return OutOfMemory();
            }
        }

        if (pCreator->m_lScriptVariablesSize)
        {
            pScriptVariables = ScriptEngine::Alloc(pCreator->m_lScriptVariablesSize, __FILE__, __LINE__);
            if (!pScriptVariables)
            {
                ScriptEngine::Free(pStateVariables);
                ScriptEngine::Free(pState);
                ScriptEngine::Free(pScriptStateInfo);
                return OutOfMemory();
            }
        }

        pState->m_pCreator = pCreator;
        pState->m_pScriptVariables = pScriptVariables;
        pState->m_pStateVariables = pStateVariables;
        pState->m_pAsyncCall = nullptr;
        pState->m_rThis = rThis;
        pState->m_pStateController = pCreator->m_pStateController;

        auto* pVariables = static_cast<LocalVarEntry*>(ScriptEngine::Alloc(sizeof(LocalVarEntry), __FILE__, __LINE__));
        pState->m_pVariables = pVariables;
        pState->m_pFunctionsVirtualTable = pCreator->m_pStateController->m_pFunctionsVirtualTable;
        if (!pVariables)
        {
            ScriptEngine::Free(pScriptVariables);
            ScriptEngine::Free(pStateVariables);
            ScriptEngine::Free(pState);
            ScriptEngine::Free(pScriptStateInfo);
            return OutOfMemory();
        }

        pState->m_Flags = 0x0224u;
        memset(pVariables, 0, sizeof(LocalVarEntry));
        pScriptStateInfo->m_pRootScriptState = pState;
        pScriptStateInfo->m_pCurrentScriptState = pState;

        return pScriptStateInfo;
    }

    const SCRIPTCREATOR* ZScriptC::FindScript(const char* pszScriptName)
    {
        if (!ScriptsPtr || !pszScriptName)
        {
            return nullptr;
        }

        const size_t lScriptNameLength = strlen(pszScriptName);
        char* pszNormalizedName = static_cast<char*>(_alloca(lScriptNameLength + 1));

        for (size_t i = 0; i < lScriptNameLength; ++i)
        {
            char c = pszScriptName[i];
            if (c == '/' || c == '\\')
            {
                c = '_';
            }

            pszNormalizedName[i] = c;
        }

        pszNormalizedName[lScriptNameLength] = '\0';

        for (uint32_t i = 1; ScriptsPtr[i]; ++i)
        {
            if (stricmp(pszNormalizedName, ScriptsPtr[i]->m_pName) == 0)
            {
                return ScriptsPtr[i];
            }
        }

        return nullptr;
    }

    ScriptState* ZScriptC::ForkState(const STATECONTROLLER* pController)
    {
        if (!pController)
        {
            return nullptr;
        }

        auto* pScheduledScript = static_cast<ZScheduledScript*>(m_pScheduleEvent);
        auto* pScriptStateInfo = static_cast<ScriptStateInfo*>(pScheduledScript->GetUserData());
        ScriptState* pRootState = pScriptStateInfo->m_pRootScriptState;
        const SCRIPTCREATOR* pRootCreator = pRootState->m_pCreator;

        // Forked scheduled scripts store their own ScriptStateInfo pair: root and current state.
        auto* pForkedScriptStateInfo = static_cast<ScriptStateInfo*>(ScriptEngine::Alloc(sizeof(ScriptStateInfo), __FILE__, __LINE__));
        if (!pForkedScriptStateInfo)
        {
            return nullptr;
        }

        auto* pForkedState = static_cast<ScriptState*>(ScriptEngine::Alloc(sizeof(ScriptState), __FILE__, __LINE__));
        if (!pForkedState)
        {
            ScriptEngine::Free(pForkedScriptStateInfo);
            return nullptr;
        }

        void* pStateVariables = ScriptEngine::Alloc(pRootCreator->m_lStateVariablesSize, __FILE__, __LINE__);
        if (!pStateVariables)
        {
            ScriptEngine::Free(pForkedState);
            ScriptEngine::Free(pForkedScriptStateInfo);
            return nullptr;
        }

        // Keep the script-global context from the root thread, but allocate a fresh state-local block.
        pForkedState->m_pScriptVariables = pRootState->m_pScriptVariables;
        pForkedState->m_pStateVariables = pStateVariables;
        pForkedState->m_pCreator = pRootCreator;
        pForkedState->m_rThis = pRootState->m_rThis;
        pForkedState->m_pStateController = pController;

        const STATECONTROLLER* pRootStateController = pRootCreator->m_pStateController;
        // State function table depends on script nesting level; PC/PS2 select root table for parent-level forks.
        pForkedState->m_pFunctionsVirtualTable =
            pController->m_lScriptLevel < pRootStateController->m_lScriptLevel ?
            pRootStateController->m_pFunctionsVirtualTable :
            pController->m_pFunctionsVirtualTable;

        uint32_t lVariablesSize = pController->m_pRun->m_lDataSize;
        if (lVariablesSize < sizeof(LocalVarEntry))
        {
            lVariablesSize = sizeof(LocalVarEntry);
        }

        auto* pVariables = static_cast<LocalVarEntry*>(ScriptEngine::Alloc(lVariablesSize, __FILE__, __LINE__));
        if (!pVariables)
        {
            ScriptEngine::Free(pStateVariables);
            ScriptEngine::Free(pForkedState);
            ScriptEngine::Free(pForkedScriptStateInfo);
            return nullptr;
        }

        pForkedState->m_pVariables = pVariables;
        pVariables->m_pFunctionController = pController->m_pRun;
        pVariables->m_lFunctionIndex = 0;
        pVariables->m_lExitFunctionIndex = 0xFFFFu;

        if (pController->m_pEnter)
        {
            auto* pFreeVariables = static_cast<LocalVarEntry*>(ScriptEngine::Alloc(sizeof(LocalVarEntry), __FILE__, __LINE__));
            if (!pFreeVariables)
            {
                ScriptEngine::Free(pVariables);
                ScriptEngine::Free(pStateVariables);
                ScriptEngine::Free(pForkedState);
                ScriptEngine::Free(pForkedScriptStateInfo);
                return nullptr;
            }

            auto* pEnterVariables = static_cast<LocalVarEntry*>(ScriptEngine::Alloc(pController->m_pEnter->m_lDataSize, __FILE__, __LINE__));
            if (!pEnterVariables)
            {
                ScriptEngine::Free(pFreeVariables);
                ScriptEngine::Free(pVariables);
                ScriptEngine::Free(pStateVariables);
                ScriptEngine::Free(pForkedState);
                ScriptEngine::Free(pForkedScriptStateInfo);
                return nullptr;
            }

            // PC inserts a cleanup frame between run and enter. After enter returns, this frame frees
            // both itself and the one-shot enter frame, then resumes from the run frame.
            pFreeVariables->m_lFunctionIndex = 0;
            pFreeVariables->m_lExitFunctionIndex = 0x7FFFu;
            pFreeVariables->m_pFunctionController = &ForkStateFree_FUNCTIONCONTROLLER;

            pEnterVariables->m_lFunctionIndex = 0;
            pEnterVariables->m_lExitFunctionIndex = 0x7FFFu;
            pEnterVariables->m_pFunctionController = pController->m_pEnter;
            pEnterVariables->m_pNextVariables = nullptr;
            pEnterVariables->m_pPrevVariables = pFreeVariables;

            pFreeVariables->m_pNextVariables = pEnterVariables;
            pFreeVariables->m_pPrevVariables = pForkedState->m_pVariables;

            pForkedState->m_pVariables->m_pNextVariables = pFreeVariables;
            pForkedState->m_pVariables = pEnterVariables;
        }

        pForkedScriptStateInfo->m_pCurrentScriptState = pForkedState;
        pForkedScriptStateInfo->m_pRootScriptState = pForkedState;

        // Fork the scheduler thread last, after all script memory has been prepared successfully.
        ZScheduledEvent* pForkedEvent = m_pScheduleEvent->Fork();
        pForkedEvent->m_pUserData = pForkedScriptStateInfo;
        pForkedState->m_pThreadInfo = pForkedEvent;

        return pForkedState;
    }

    ScriptState* ZScriptC::GetRootScriptState()
    {
        auto* pScheduledScript = static_cast<ZScheduledScript*>(m_pScheduleEvent);
        if (!pScheduledScript)
        {
            return nullptr;
        }

        auto* pScriptStateInfo = static_cast<ScriptStateInfo*>(pScheduledScript->GetUserData());
        if (pScriptStateInfo)
        {
            return pScriptStateInfo->m_pRootScriptState;
        }

        return nullptr;
    }

    bool ZScriptC::TerminateScript()
    {
        if (auto* pScript = GetSchedEvent())
        {
            const bool bFreed = FreeThread((ScriptStateInfo*)pScript->GetUserData());
            if (bFreed)
            {
                DeactivateFrameUpdate();
                DeactivateScheduleUpdate();
                m_lRoutCases &= ~0x20u;

                Remove();
            }

            return bFreed;
        }

        return false;
    }

    bool ZScriptC::FreeThread(ScriptStateInfo* pState)
    {
        bool bFreedScriptVariables = false;

        if (pState)
        {
            ScriptState* pRootState = pState->m_pRootScriptState;
            ScriptEngine::Free(pRootState->m_pStateVariables);

            auto* pScheduledScript = static_cast<ZScheduledScript*>(m_pScheduleEvent);
            if (!pScheduledScript->HasThreads())
            {
                ScriptEngine::Free(pRootState->m_pScriptVariables);
                pRootState->m_pScriptVariables = nullptr;
                bFreedScriptVariables = true;
            }

            for (ScriptState* pCurrentState = pRootState; pCurrentState; )
            {
                LocalVarEntry* pVariables = pCurrentState->m_pVariables;
                ScriptState* pAlienState = pCurrentState->m_pAlienCall;

                // Async calls own a temporary LocalVarEntry frame that must be released with the queue node.
                while (pCurrentState->m_pAsyncCall)
                {
                    AsyncCall_Struct* pAsyncCall = pCurrentState->m_pAsyncCall;
                    pCurrentState->m_pAsyncCall = pAsyncCall->pNext;
                    ScriptEngine::Free(pAsyncCall->m_pLVE);
                    ScriptEngine::Free(pAsyncCall);
                }

                // Queued messages store their payload inline, so freeing the queue node frees the payload too.
                for (MessageCue* pMessageCue = pCurrentState->m_pMessageCue; pMessageCue; )
                {
                    MessageCue* pNextMessageCue = pMessageCue->m_pNext;
                    ScriptEngine::Free(pMessageCue);
                    pMessageCue = pNextMessageCue;
                }

                pCurrentState->m_Flags &= 0x0FFFu;
                pCurrentState->m_pMessageCue = nullptr;

                if (pVariables)
                {
                    // The current frame can be in the middle of the LocalVarEntry chain; PC rewinds to the tail
                    // through m_pPrevVariables, then frees forward through m_pNextVariables.
                    while (pVariables->m_pPrevVariables)
                    {
                        pVariables = pVariables->m_pPrevVariables;
                    }

                    while (pVariables)
                    {
                        LocalVarEntry* pNextVariables = pVariables->m_pNextVariables;
                        if (pNextVariables)
                        {
                            pNextVariables->m_pPrevVariables = pVariables->m_pPrevVariables;
                        }

                        pVariables->m_pNextVariables = nullptr;
                        ScriptEngine::Free(pVariables);
                        pVariables = pNextVariables;
                    }
                }

                pCurrentState->m_pVariables = nullptr;
                ScriptEngine::Free(pCurrentState);
                pCurrentState = pAlienState;
            }

            pScheduledScript->SetUserData(nullptr);
            ScriptEngine::Free(pState);
        }

        return bFreedScriptVariables;
    }

    int ZScriptC::GetNrThreads()
    {
        ZScheduledEvent* pFirstThread = m_pScheduleEvent;
        ZScheduledEvent* pThread = pFirstThread;
        int lThreadCount = 0;

        do
        {
            if (!pThread)
            {
                break;
            }

            ++lThreadCount;
            pThread = pThread->GetNextThread();
        }
        while (pThread != pFirstThread);

        return lThreadCount;
    }

    const ZScheduledScript* ZScriptC::GetSchedEvent()
    {
        return reinterpret_cast<const ZScheduledScript*>(m_pScheduleEvent);
    }

    TIMETYPE ZScriptC::GetNextRun()
    {
        return GetSchedEvent()->m_tNextRun;
    }

    int ZScriptC::GetPriority()
    {
        return GetSchedEvent()->GetPriority();
    }

    void ZScriptC::NukeAndRestart()
    {
        // I guess, NukeAndRestart concept does not exists in PC build
        // Uncomment next line if we will find it in PC
        // s_pStringMap = nullptr;
    }

    void ZScriptC::GetName(ZRTString& name)
    {
        // Do nothing
    }

    void ZScriptC::SetName(const ZRTString& name)
    {
        const char* pszScriptCModule = nullptr;
        if (g_pSysInterface && g_pSysInterface->m_pEngineData)
        {
            CCom* pSceneCom = g_pSysInterface->m_pEngineData->GetSceneCom();
            pszScriptCModule = pSceneCom->GetVal("ScriptCModule");
        }

        if (!pszScriptCModule)
        {
            return;
        }

        MYSTR sScriptPath;
        char* pszOverrideScriptPath = nullptr;
        if (g_pSysInterface->GetOption("OverrideScriptPath", &pszOverrideScriptPath))
        {
            sScriptPath += MYSTR(pszOverrideScriptPath);
            sScriptPath += MYSTR("\\");
            sScriptPath += MYSTR(pszScriptCModule);
        }
        else
        {
            sScriptPath = g_pSysInterface->ProjectPath();
            sScriptPath += MYSTR("scriptcs/_gamerelease/");
            sScriptPath += MYSTR(pszScriptCModule);
        }

        char* pszDisableScripts = nullptr;
        if (g_pSysInterface->GetOption("DisableScripts", &pszDisableScripts) && std::atof(pszDisableScripts) != 0.0)
        {
            m_pScriptCreator = nullptr;
            return;
        }

        MYSTR sDllPath = sScriptPath + MYSTR(".dll");
        if (ScriptEngine::AttachSceneScripts(sDllPath))
        {
            m_pScriptCreator = FindScript(name.c_str());
        }
    }


    STATIC_CLASS_VAR_IMPL(ZScriptC, RTP::ZPropertyInfo, Info, 0x00811984, {});
}
