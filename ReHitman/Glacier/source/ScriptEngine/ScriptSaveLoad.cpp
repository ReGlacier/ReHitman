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

        void* FieldToSaveForm(void* ptr)
        {
            return reinterpret_cast<void*>((*g_pSavedPointersMap)[ptr]);
        }

        void* FieldFromSaveForm(uint32_t lIndex)
        {
            return (*g_pSaveTable)[lIndex].m_pAddr;
        }
    }

    // In-place counterpart of FixupLoadedData: bEncode == false converts the block into save form
    // (script-code pointers become high-bit offsets, saved pointers become save-table indices),
    // bEncode == true restores the original pointers after the table was streamed out.
    void FixupData(uint16_t* pStringOffsets, int8_t* pBase, bool bEncode, bool /*bTopLevel*/)
    {
        if (!pBase || !pStringOffsets)
        {
            return;
        }

        const uint32_t length = ScriptEngine::AllocSize(pBase);
        for (uint16_t* pCurrent = pStringOffsets; *pCurrent; )
        {
            const uint16_t type = *pCurrent++;
            const uint32_t start = *pCurrent++;
            const uint32_t end = *pCurrent++;

            ZASSERT(start < length);
            ZASSERT(end <= length);

            auto* pStart = reinterpret_cast<uint32_t*>(pBase + start);
            auto* pEnd = reinterpret_cast<uint32_t*>(pBase + end);

            switch (type)
            {
                case SRT_VARIABLES:
                    if (!bEncode)
                    {
                        for (auto* pValue = pStart; pValue < pEnd; ++pValue)
                        {
                            const uint32_t value = *pValue;
                            if (static_cast<int32_t>(value) < 0)
                            {
                                *pValue = ScriptEngine::GetOffsetInScriptCode(reinterpret_cast<void*>(value));
                            }
                            else if (value)
                            {
                                *pValue = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(FieldToSaveForm(reinterpret_cast<void*>(value))));
                            }
                        }
                    }
                    else
                    {
                        for (auto* pValue = pStart; pValue < pEnd; ++pValue)
                        {
                            const uint32_t value = *pValue;
                            if (static_cast<int32_t>(value) < 0)
                            {
                                *pValue = reinterpret_cast<uint32_t>(ScriptEngine::GetAddressInScriptCode(value));
                            }
                            else
                            {
                                *pValue = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(FieldFromSaveForm(value)));
                            }
                        }
                    }
                    break;

                case SRT_SCRIPTSTATE:
                    if (bEncode)
                    {
                        for (auto* pValue = pStart; pValue < pEnd; ++pValue)
                        {
                            *pValue = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(FieldFromSaveForm(*pValue)));
                        }
                    }
                    break;

                case SRT_SCRIPTVARIABLES:
                case SRT_DYNSTRING:
                    if (!bEncode)
                    {
                        for (auto* pValue = pStart; pValue < pEnd; ++pValue)
                        {
                            *pValue = ScriptEngine::GetOffsetInScriptCode(reinterpret_cast<void*>(*pValue));
                        }
                    }
                    else
                    {
                        for (auto* pValue = pStart; pValue < pEnd; ++pValue)
                        {
                            *pValue = reinterpret_cast<uint32_t>(ScriptEngine::GetAddressInScriptCode(*pValue));
                        }
                    }
                    break;

                case SRT_STATEVARIABLES:
                    if (bEncode)
                    {
                        for (auto* pValue = pStart; pValue < pEnd; pValue += 2)
                        {
                            if (pValue[1])
                            {
                                auto* pEntry = (*g_pSaveTable)[pValue[1]].m_pExtra;
                                const auto baseIndex = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(pEntry));
                                const auto offset = static_cast<uint32_t>(reinterpret_cast<uintptr_t>((*g_pSaveTable)[baseIndex].m_pExtra));
                                pValue[1] = reinterpret_cast<uint32_t>(static_cast<int8_t*>((*g_pSaveTable)[pValue[1]].m_pAddr) + offset);
                            }
                        }
                    }
                    break;

                case SRT_ASYNCCALL_STRUCT:
                {
                    auto* pAsyncCallData = pStart;
                    if (bEncode && pAsyncCallData[5])
                    {
                        pAsyncCallData[5] = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(FieldFromSaveForm(pAsyncCallData[5])));
                    }

                    pAsyncCallData[7] = bEncode
                        ? reinterpret_cast<uint32_t>(ScriptEngine::GetAddressInScriptCode(pAsyncCallData[7]))
                        : ScriptEngine::GetOffsetInScriptCode(reinterpret_cast<void*>(pAsyncCallData[7]));
                    pAsyncCallData[8] = bEncode
                        ? reinterpret_cast<uint32_t>(ScriptEngine::GetAddressInScriptCode(pAsyncCallData[8]))
                        : ScriptEngine::GetOffsetInScriptCode(reinterpret_cast<void*>(pAsyncCallData[8]));
                }
                break;

                default:
                    break;
            }
        }
    }

    void FixupSaveTable(bool bEncode)
    {
        const uint32_t lEntryCount = static_cast<uint32_t>(g_pSaveTable->size());
        for (uint32_t i = 0; i < lEntryCount; ++i)
        {
            SaveRefEntry& entry = (*g_pSaveTable)[i];

            switch (entry.m_srt)
            {
                case SRT_VARIABLES:
                {
                    auto* pVar = static_cast<LocalVarEntry*>(entry.m_pAddr);
                    if (!bEncode)
                    {
                        pVar->m_pFunctionController = reinterpret_cast<const FUNCTIONCONTROLLER*>(static_cast<uintptr_t>(ScriptEngine::GetOffsetInScriptCode(pVar->m_pFunctionController)));
                        pVar->m_pNextVariables = static_cast<LocalVarEntry*>(FieldToSaveForm(pVar->m_pNextVariables));
                        pVar->m_pPrevVariables = static_cast<LocalVarEntry*>(FieldToSaveForm(pVar->m_pPrevVariables));
                        FixupData(static_cast<uint16_t*>(const_cast<void*>(entry.m_pExtra)), reinterpret_cast<int8_t*>(pVar), false, true);
                    }
                    else
                    {
                        pVar->m_pFunctionController = static_cast<const FUNCTIONCONTROLLER*>(ScriptEngine::GetAddressInScriptCode(reinterpret_cast<uint32_t>(const_cast<FUNCTIONCONTROLLER*>(pVar->m_pFunctionController))));
                        pVar->m_pNextVariables = static_cast<LocalVarEntry*>(FieldFromSaveForm(reinterpret_cast<uint32_t>(pVar->m_pNextVariables)));
                        pVar->m_pPrevVariables = static_cast<LocalVarEntry*>(FieldFromSaveForm(reinterpret_cast<uint32_t>(pVar->m_pPrevVariables)));

                        if (entry.m_pExtra)
                        {
                            FixupData(static_cast<uint16_t*>(const_cast<void*>(entry.m_pExtra)), reinterpret_cast<int8_t*>(pVar), true, true);
                        }
                    }
                }
                break;

                case SRT_SCRIPTSTATE:
                {
                    auto* pState = static_cast<ScriptState*>(entry.m_pAddr);
                    if (!pState)
                    {
                        break;
                    }

                    if (!bEncode)
                    {
                        pState->m_pAlienCall = static_cast<ScriptState*>(FieldToSaveForm(pState->m_pAlienCall));
                        pState->m_pVariables = static_cast<LocalVarEntry*>(FieldToSaveForm(pState->m_pVariables));
                        pState->m_pStateVariables = FieldToSaveForm(pState->m_pStateVariables);
                        pState->m_pScriptVariables = FieldToSaveForm(pState->m_pScriptVariables);
                        pState->m_pFunctionsVirtualTable = reinterpret_cast<const void*>(static_cast<uintptr_t>(ScriptEngine::GetOffsetInScriptCode(pState->m_pFunctionsVirtualTable)));
                        pState->m_pStateController = reinterpret_cast<const STATECONTROLLER*>(static_cast<uintptr_t>(ScriptEngine::GetOffsetInScriptCode(pState->m_pStateController)));
                        pState->m_pNextStateController = reinterpret_cast<const STATECONTROLLER*>(static_cast<uintptr_t>(ScriptEngine::GetOffsetInScriptCode(pState->m_pNextStateController)));
                        pState->m_pPreviousStateController = reinterpret_cast<const STATECONTROLLER*>(static_cast<uintptr_t>(ScriptEngine::GetOffsetInScriptCode(pState->m_pPreviousStateController)));
                        pState->m_pCreator = reinterpret_cast<const SCRIPTCREATOR*>(static_cast<uintptr_t>(ScriptEngine::GetOffsetInScriptCode(pState->m_pCreator)));
                        pState->m_pAsyncCall = static_cast<AsyncCall_Struct*>(FieldToSaveForm(pState->m_pAsyncCall));
                        if (pState->m_pAsyncCall)
                        {
                            pState->m_pAsyncCallLast = static_cast<AsyncCall_Struct*>(FieldToSaveForm(pState->m_pAsyncCallLast));
                        }
                        else
                        {
                            pState->m_pAsyncCallLast = nullptr;
                        }

                        pState->m_pMessageCue = static_cast<MessageCue*>(FieldToSaveForm(pState->m_pMessageCue));
                    }
                    else
                    {
                        pState->m_pAlienCall = static_cast<ScriptState*>(FieldFromSaveForm(reinterpret_cast<uint32_t>(pState->m_pAlienCall)));
                        pState->m_pVariables = static_cast<LocalVarEntry*>(FieldFromSaveForm(reinterpret_cast<uint32_t>(pState->m_pVariables)));
                        pState->m_pStateVariables = FieldFromSaveForm(reinterpret_cast<uint32_t>(pState->m_pStateVariables));
                        pState->m_pScriptVariables = FieldFromSaveForm(reinterpret_cast<uint32_t>(pState->m_pScriptVariables));
                        pState->m_pFunctionsVirtualTable = ScriptEngine::GetAddressInScriptCode(reinterpret_cast<uint32_t>(const_cast<void*>(pState->m_pFunctionsVirtualTable)));
                        pState->m_pStateController = static_cast<const STATECONTROLLER*>(ScriptEngine::GetAddressInScriptCode(reinterpret_cast<uint32_t>(const_cast<STATECONTROLLER*>(pState->m_pStateController))));
                        pState->m_pNextStateController = static_cast<const STATECONTROLLER*>(ScriptEngine::GetAddressInScriptCode(reinterpret_cast<uint32_t>(const_cast<STATECONTROLLER*>(pState->m_pNextStateController))));
                        pState->m_pPreviousStateController = static_cast<const STATECONTROLLER*>(ScriptEngine::GetAddressInScriptCode(reinterpret_cast<uint32_t>(const_cast<STATECONTROLLER*>(pState->m_pPreviousStateController))));
                        pState->m_pCreator = static_cast<const SCRIPTCREATOR*>(ScriptEngine::GetAddressInScriptCode(reinterpret_cast<uint32_t>(const_cast<SCRIPTCREATOR*>(pState->m_pCreator))));
                        pState->m_pAsyncCall = static_cast<AsyncCall_Struct*>(FieldFromSaveForm(reinterpret_cast<uint32_t>(pState->m_pAsyncCall)));
                        if (pState->m_pAsyncCall)
                        {
                            pState->m_pAsyncCallLast = static_cast<AsyncCall_Struct*>(FieldFromSaveForm(reinterpret_cast<uint32_t>(pState->m_pAsyncCallLast)));
                        }
                        else
                        {
                            pState->m_pAsyncCallLast = nullptr;
                        }

                        pState->m_pMessageCue = static_cast<MessageCue*>(FieldFromSaveForm(reinterpret_cast<uint32_t>(pState->m_pMessageCue)));
                    }
                }
                break;

                case SRT_SCRIPTVARIABLES:
                    for (auto* pController = static_cast<const STATECONTROLLER*>(entry.m_pExtra); pController; pController = pController->m_pParent)
                    {
                        FixupData(pController->m_lStringOffsets, static_cast<int8_t*>(entry.m_pAddr), bEncode, false);
                    }
                    break;

                case SRT_STATEVARIABLES:
                    for (auto* pController = static_cast<const STATECONTROLLER*>(entry.m_pExtra); pController && pController->m_lScriptLevel > 1; pController = pController->m_pParent)
                    {
                        FixupData(pController->m_lStringOffsets, static_cast<int8_t*>(entry.m_pAddr), bEncode, false);
                    }
                    break;

                case SRT_ASYNCCALL_STRUCT:
                {
                    auto* pAsyncCall = static_cast<AsyncCall_Struct*>(entry.m_pAddr);
                    if (!pAsyncCall)
                    {
                        break;
                    }

                    if (!bEncode)
                    {
                        pAsyncCall->pNext = static_cast<AsyncCall_Struct*>(FieldToSaveForm(pAsyncCall->pNext));
                        pAsyncCall->m_pLVE = static_cast<LocalVarEntry*>(FieldToSaveForm(pAsyncCall->m_pLVE));
                    }
                    else
                    {
                        pAsyncCall->pNext = static_cast<AsyncCall_Struct*>(FieldFromSaveForm(reinterpret_cast<uint32_t>(pAsyncCall->pNext)));
                        pAsyncCall->m_pLVE = static_cast<LocalVarEntry*>(FieldFromSaveForm(reinterpret_cast<uint32_t>(pAsyncCall->m_pLVE)));
                    }
                }
                break;

                case SRT_ENTERS:
                {
                    auto* pRefs = static_cast<uint32_t*>(entry.m_pAddr);
                    if (!pRefs)
                    {
                        break;
                    }

                    const uint32_t lRefCount = entry.m_lSize / sizeof(uint32_t);
                    for (uint32_t j = 0; j < lRefCount; ++j)
                    {
                        pRefs[j] = bEncode
                            ? reinterpret_cast<uint32_t>(ScriptEngine::GetAddressInScriptCode(pRefs[j]))
                            : ScriptEngine::GetOffsetInScriptCode(reinterpret_cast<void*>(pRefs[j]));
                    }
                }
                break;

                case SRT_MESSAGECUE:
                {
                    auto* pMessageCue = static_cast<MessageCue*>(entry.m_pAddr);
                    if (!pMessageCue)
                    {
                        break;
                    }

                    if (!bEncode)
                    {
                        pMessageCue->m_pLast = static_cast<MessageCue*>(FieldToSaveForm(pMessageCue->m_pLast));
                        pMessageCue->m_pNext = static_cast<MessageCue*>(FieldToSaveForm(pMessageCue->m_pNext));
                    }
                    else
                    {
                        pMessageCue->m_pLast = static_cast<MessageCue*>(FieldFromSaveForm(reinterpret_cast<uint32_t>(pMessageCue->m_pLast)));
                        pMessageCue->m_pNext = static_cast<MessageCue*>(FieldFromSaveForm(reinterpret_cast<uint32_t>(pMessageCue->m_pNext)));
                    }
                }
                break;

                case SRT_NULL:
                case SRT_DYNSTRING:
                case SRT_EVENTREF:
                    break;

                default:
                    ZASSERT(false);
                    break;
            }
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
