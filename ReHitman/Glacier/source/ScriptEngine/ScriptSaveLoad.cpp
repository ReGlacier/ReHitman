#include <Glacier/ScriptEngine/ScriptSaveLoad.h>
#include <Glacier/ScriptEngine/AsyncCall_Struct.h>
#include <Glacier/ScriptEngine/FUNCTIONCONTROLLER.h>
#include <Glacier/ScriptEngine/Globals.h>
#include <Glacier/ScriptEngine/LocalVarEntry.h>
#include <Glacier/ScriptEngine/MessageCue.h>
#include <Glacier/ScriptEngine/ScriptEngine.h>
#include <Glacier/ScriptEngine/ScriptState.h>
#include <Glacier/ScriptEngine/SCRIPTCREATOR.h>
#include <Glacier/ScriptEngine/STATECONTROLLER.h>
#include <Glacier/ScriptEngine/ZScriptC.h>
#include <Glacier/ZSTL/ISaveMemoryManager.h>
#include <Glacier/ZUniMemory.h>
#include <cstring>


namespace Glacier
{
    namespace
    {
        uint32_t SaveTableIndex(void* ptr)
        {
            return (*g_pSavedPointersMap)[ptr];
        }

        uint32_t PushSaveRef(const SaveRefEntry& entry)
        {
            const uint32_t index = static_cast<uint32_t>(g_pSaveTable->size());
            g_pSaveTable->push_back(entry);
            return index;
        }
    }

    void PrepareSave(_ScriptState* pScriptState)
    {
        if (!g_pSavedPointersMap)
        {
            auto* pMapMemory = static_cast<SaveTableMap*>(ISaveMemoryManager::Alloc(sizeof(SaveTableMap)));
            auto* pTableMemory = static_cast<SaveTableVector*>(ISaveMemoryManager::Alloc(sizeof(SaveTableVector)));

            g_pSavedPointersMap = znew_placement<SaveTableMap>(pMapMemory);
            (*g_pSavedPointersMap)[nullptr] = 0;

            g_pSaveTable = znew_placement<SaveTableVector>(pTableMemory);
            g_pSaveTable->push_back({});
        }

        PrepareScriptState(pScriptState);
    }
    
    void PrepareScriptState(_ScriptState* pScriptState)
    {
        if (!pScriptState)
        {
            return;
        }

        if (SaveTableIndex(pScriptState))
        {
            return;
        }

        const uint32_t index = PushSaveRef({ pScriptState, sizeof(ScriptState), SRT_SCRIPTSTATE, nullptr });
        (*g_pSavedPointersMap)[pScriptState] = index;

        PrepareScriptState(pScriptState->m_pAlienCall);
        PrepareVariables(pScriptState->m_pVariables);
        PrepareAsyncCall(pScriptState->m_pAsyncCall);

        if (pScriptState->m_pAsyncCall)
        {
            PrepareAsyncCall(pScriptState->m_pAsyncCallLast);
        }

        PrepareMessageCue(pScriptState->m_pMessageCue);
        PrepareStateVariables(pScriptState);
        PrepareScriptVariables(pScriptState);
    }
    
    void PrepareScriptVariables(_ScriptState* pScript)
    {
        void** ppScriptVariables = &pScript->m_pScriptVariables;
        if (!*ppScriptVariables || SaveTableIndex(ppScriptVariables))
        {
            return;
        }

        const STATECONTROLLER* pStateController = pScript->m_pCreator->m_pStateController;
        const SaveRefEntry entry
        {
            *ppScriptVariables,
            static_cast<uint32_t>(pScript->m_pCreator->m_lScriptVariablesSize),
            SRT_SCRIPTVARIABLES,
            pStateController
        };

        const uint32_t index = PushSaveRef(entry);
        (*g_pSavedPointersMap)[ppScriptVariables] = index;

        for (; pStateController; pStateController = pStateController->m_pParent)
        {
            PrepareData(pStateController->m_lStringOffsets, static_cast<int8_t*>(entry.m_pAddr));
        }
    }
    
    void PrepareStateVariables(_ScriptState* pScriptState)
    {
        void** ppStateVariables = &pScriptState->m_pStateVariables;
        if (!*ppStateVariables || SaveTableIndex(ppStateVariables))
        {
            return;
        }

        const STATECONTROLLER* pStateController = pScriptState->m_pStateController;
        const SaveRefEntry entry
        {
            *ppStateVariables,
            static_cast<uint32_t>(pScriptState->m_pCreator->m_lStateVariablesSize),
            SRT_STATEVARIABLES,
            pStateController
        };

        const uint32_t index = PushSaveRef(entry);
        (*g_pSavedPointersMap)[ppStateVariables] = index;

        for (; pStateController && pStateController->m_lScriptLevel >= 2; pStateController = pStateController->m_pParent)
        {
            PrepareData(pStateController->m_lStringOffsets, static_cast<int8_t*>(entry.m_pAddr));
        }
    }
    
    void PrepareData(uint16_t* pStringOffsets, int8_t* pBase)
    {
        if (!pBase || !pStringOffsets)
        {
            return;
        }

        const uint32_t length = ScriptEngine::AllocSize(pBase);
        for (uint16_t* pCurrent = pStringOffsets; *pCurrent;)
        {
            const uint16_t type = *pCurrent++;
            const uint32_t start = *pCurrent++;
            const uint32_t end = *pCurrent++;

            ZASSERT(start < length);
            ZASSERT(end <= length);

            int8_t* pStart = pBase + start;
            int8_t* pEnd = pBase + end;

            if (type == SRT_VARIABLES)
            {
                while (pStart < pEnd)
                {
                    auto*& pszString = *reinterpret_cast<char**>(pStart);
                    if (pszString && !ScriptEngine::IsScriptCode(pszString))
                    {
                        const uint32_t index = PushSaveRef({ pszString, static_cast<uint32_t>(std::strlen(pszString) + 1), SRT_DYNSTRING, nullptr });
                        (*g_pSavedPointersMap)[pszString] = index;
                    }

                    pStart += sizeof(char*);
                }
            }
            else if (type == SRT_SCRIPTSTATE)
            {
                while (pStart < pEnd)
                {
                    auto*& pScriptState = *reinterpret_cast<ScriptState**>(pStart);
                    if (s_pCurrentSaveGameObject->IsValidThread(pScriptState))
                    {
                        PrepareScriptState(pScriptState);
                    }
                    else
                    {
                        pScriptState = nullptr;
                    }

                    pStart += sizeof(ScriptState*);
                }
            }
            else if (type == SRT_EVENTREF)
            {
                while (pStart < pEnd)
                {
                    auto* pEventRef = reinterpret_cast<void**>(pStart);
                    if (pEventRef[1])
                    {
                        const auto offset = static_cast<uint32_t>(reinterpret_cast<int8_t*>(pEventRef[1]) - pBase);
                        if (reinterpret_cast<int8_t*>(pEventRef[1]) >= pBase && offset < length)
                        {
                            const uint32_t baseIndex = SaveTableIndex(pBase);
                            const uint32_t index = PushSaveRef({ reinterpret_cast<void*>(offset), 0, SRT_EVENTREF, reinterpret_cast<void*>(baseIndex) });
                            (*g_pSavedPointersMap)[&pEventRef[1]] = index;
                        }
                    }

                    pStart += sizeof(void*) * 2;
                }
            }
            else if (type == SRT_ASYNCCALL_STRUCT)
            {
                auto* pAsyncCallField = reinterpret_cast<AsyncCall_Struct**>(pStart + 20);
                if (*pAsyncCallField && !SaveTableIndex(*pAsyncCallField))
                {
                    const uint32_t index = PushSaveRef({ *pAsyncCallField, ScriptEngine::AllocSize(*pAsyncCallField), SRT_ENTERS, nullptr });
                    (*g_pSavedPointersMap)[*pAsyncCallField] = index;
                }
            }
        }
    }
    
    void PrepareAsyncCall(_AsyncCall_Struct* pAsyncCall)
    {
        if (!pAsyncCall || SaveTableIndex(pAsyncCall))
        {
            return;
        }

        const uint32_t index = PushSaveRef({ pAsyncCall, ScriptEngine::AllocSize(pAsyncCall), SRT_ASYNCCALL_STRUCT, nullptr });
        (*g_pSavedPointersMap)[pAsyncCall] = index;

        PrepareAsyncCall(pAsyncCall->pNext);
        PrepareVariables(pAsyncCall->m_pLVE);
    }
    
    void PrepareMessageCue(_MessageCue* pQue)
    {
        if (!pQue || SaveTableIndex(pQue))
        {
            return;
        }

        const uint32_t index = PushSaveRef({ pQue, ScriptEngine::AllocSize(pQue), SRT_MESSAGECUE, nullptr });
        (*g_pSavedPointersMap)[pQue] = index;

        PrepareMessageCue(pQue->m_pLast);
        PrepareMessageCue(pQue->m_pNext);
    }
    
    void PrepareVariables(_LocalVarEntry* pVar)
    {
        if (!pVar || SaveTableIndex(pVar))
        {
            return;
        }

        const uint32_t length = ScriptEngine::AllocSize(pVar);
        ZASSERT(length < 0x10000);

        const FUNCTIONCONTROLLER* pFunctionController = pVar->m_pFunctionController;
        const SaveRefEntry entry
        {
            pVar,
            length,
            SRT_VARIABLES,
            pFunctionController ? pFunctionController->m_lStringOffsets : nullptr
        };

        const uint32_t index = PushSaveRef(entry);
        (*g_pSavedPointersMap)[pVar] = index;

        PrepareVariables(pVar->m_pNextVariables);
        PrepareVariables(pVar->m_pPrevVariables);

        if (pFunctionController)
        {
            PrepareData(pFunctionController->m_lStringOffsets, static_cast<int8_t*>(entry.m_pAddr));
        }
    }
    
}
