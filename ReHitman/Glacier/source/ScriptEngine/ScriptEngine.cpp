#include <Glacier/ScriptEngine/ScriptEngine.h>
#include <Glacier/ScriptEngine/FUNCTIONCONTROLLER.h>
#include <Glacier/ScriptEngine/ZScriptImportTable.h>
#include <Glacier/ScriptEngine/ZScriptC_ZMessage.h>
#include <Glacier/ScriptEngine/SCRIPTFUNCTIONS.h>
#include <Glacier/ScriptEngine/SCRIPTCREATOR.h>
#include <Glacier/ScriptEngine/LocalVarEntry.h>
#include <Glacier/ScriptEngine/ScriptState.h>
#include <Glacier/ScriptEngine/Globals.h>
#include <Glacier/ScriptEngine/SF.h>
#include <Glacier/EventBase/ZScheduledScript.h>
#include <Glacier/EventBase/ZEventBuffer.h>
#include <Glacier/Filesystem/ZSysFile.h>
#include <Glacier/ZSTL/ZMallocSimple.h>
#include <Glacier/ZSTL/ZOffsetAlloc.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/ZUniMemory.h>
#include <Windows.h>


namespace Glacier
{
    namespace
    {
        // Case-insensitive compare with the original quirks (PC sub_549B50):
        // both chars are folded with & 0xDF (ASCII uppercase) and 0x0E maps to '_'.
        static bool strscriptccmp(const char* pStr, const char* pName)
        {
            auto fold = [](char c) -> char
            {
                char v = c & 0xDF;
                return v == 0x0E ? '_' : v;
            };

            if (!*pStr)
            {
                return true;
            }

            for (;;)
            {
                if (fold(*pStr) != fold(*pName))
                {
                    return false;
                }
                ++pStr;
                ++pName;
                if (!*pStr)
                {
                    return true;
                }
            }
        }
    }
    
    STATIC_CLASS_VAR_IMPL(ScriptEngine, _SpecialScriptReturnType, m_SpecialScriptReturnType, 0x008289A8, {});

    void InitializeScriptFunctions(_SCRIPTFUNCTIONS* pSF)
    {
        // Order follows the PC binary (InitializeScriptFunctions at 0x0043BE50).
        g_SF = pSF;

        pSF->CheckTimeout = SF_CheckTimeout;                             // 0x18
        pSF->DebugPrint = SF_DebugPrint;                                 // 0x24
        pSF->Sleep = ScriptEngine::Sleep;                                // 0x08
        pSF->Input = SF_Input;                                           // 0x30
        pSF->Pack = SF_Pack;                                             // 0x28
        pSF->Unpack = SF_Unpack;                                         // 0x2C
        pSF->GetZDefine = SF_GetZDefine;                                 // 0x34
        pSF->Alloc = ScriptEngine::Alloc;                                // 0x38
        pSF->AllocNM = ScriptEngine::AllocNM;                            // 0x3C
        pSF->Free = ScriptEngine::Free;                                  // 0x40
        pSF->FreeNM = ScriptEngine::FreeNM;                              // 0x44
        pSF->RunNoBreak = ScriptEngine::RunNoBreak;                      // 0x48
        pSF->GetAlienVirtualTableEntry = ScriptEngine::GetAlienVirtualTableEntry; // 0x50
        pSF->GetAlienScriptState = ScriptEngine::GetAlienScriptState;    // 0x54
        pSF->GetRootScriptStateRef = ScriptEngine::GetRootScriptStateRef;// 0x58
        pSF->FindScriptStateByRef = ScriptEngine::FindScriptStateByRef;  // 0x4C
        pSF->SetForkStateController = ScriptEngine::SetForkStateController; // 0x00
        pSF->GetForkThread = ScriptEngine::GetForkThread;                // 0x04
        pSF->ResumeThread = ScriptEngine::ResumeThread;                  // 0x14
        pSF->SendCommand = ScriptEngine::SendCommand;                    // 0x1C
        pSF->SendScriptCommand = ScriptEngine::SendScriptCommand;        // 0x20
        pSF->StopThread = ScriptEngine::StopThread;                      // 0x0C
        pSF->TerminateThread = ScriptEngine::TerminateThread;            // 0x10
        pSF->GetPriority = ScriptEngine::GetPriority;                    // 0x64
        pSF->SetPriority = ScriptEngine::SetPriority;                    // 0x68
        pSF->Memcpy = SF_Memcpy;                                         // 0x5C
        pSF->Memset = SF_Memset;                                         // 0x60
    }

    void* ScriptEngine::Alloc(uint32_t lSize, const char* psFile, uint32_t lLine)
    {
        if (g_pMessageAllocator)
        {
            // ZOffsetAlloc::Alloc returns a pool offset; the block address is
            // the allocator base + that offset. Size (with the 4-byte header)
            // is stored in the header, payload follows it.
            void* lOffset = g_pMessageAllocator->Alloc(lSize + 4, true);
            if (lOffset == reinterpret_cast<void*>(-1))
            {
                ZASSERT(false);
                return nullptr;
            }

            auto* pBlock = reinterpret_cast<uint32_t*>(
                reinterpret_cast<char*>(g_pMessageAllocator) + reinterpret_cast<uintptr_t>(lOffset));
            *pBlock = lSize + 4;

            char* pPayload = reinterpret_cast<char*>(pBlock + 1);
            memset(pPayload, 0, lSize);
            return pPayload;
        }

        // Fallback to generic allocator
        char* pBeginBlock = nullptr;
        char* pEndBlock = nullptr;
        char* pAllocated = g_ScriptAllocator.Alloc(lSize, pBeginBlock, pEndBlock);
        if (pAllocated)
        {
            memset(pAllocated, 0, lSize);
        }

        return (void*)pAllocated;
    }

    void* ScriptEngine::AllocNM(uint32_t lSize, const char* psFile, uint32_t lLine)
    {
        char* bBlockStart { nullptr };
        char* pBlockEnd { nullptr };

        char* ptr = g_ScriptAllocator.Alloc(lSize, bBlockStart, pBlockEnd);
        if (ptr)
        {
            memset(ptr, 0x0, lSize);
        }

        return (void*)ptr;
    }

    uint32_t ScriptEngine::AllocSize(void* ptr)
    {
        return g_ScriptAllocator.AllocSize((char*)ptr);
    }

    bool ScriptEngine::AttachSceneScripts(const char* pszSceneName)
    {
        if (lScriptLoadedCount)
        {
            ++lScriptLoadedCount;
            return g_bScriptLoadResult;
        }

        MYSTR sSceneFilename = g_pSysFile->ConvertFilename(pszSceneName);
        const char* pszSceneScriptDllName = sSceneFilename;

        g_pScripts = (void*)LoadLibraryA(pszSceneScriptDllName);
        if (!g_pScripts)
        {
            const auto dwLastError = GetLastError();
            char* pszBuffer = nullptr;

            const DWORD dwFormatResult = FormatMessageA(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                dwLastError,
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                reinterpret_cast<LPSTR>(&pszBuffer),
                0,
                NULL
            );

            if (dwFormatResult > 0 && pszBuffer)
            {
                printf("[ScriptEngine::AttachSceneScripts] Failed to load library '%s'. Error code: %lu (0x%X) - %s", 
                    pszSceneScriptDllName, 
                    dwLastError, 
                    dwLastError, 
                    pszBuffer);
                
                LocalFree(pszBuffer);
            }
            else
            {
                printf("[ScriptEngine::AttachSceneScripts] Failed to load library '%s'. Error code: %lu (0x%X)\n",
                    pszSceneScriptDllName, 
                    dwLastError,
                    dwLastError);
            }

            g_bScriptLoadResult = false;
            return g_bScriptLoadResult;
        }

        ++lScriptLoadedCount;
        znew_placement<ZMallocSimple>(&g_ScriptAllocator);
        g_ScriptAllocator.AddBlock(g_ScriptMemory, 0x80000u);

        InitializeScriptFunctions(
            reinterpret_cast<SCRIPTFUNCTIONS*>(
                GetProcAddress(
                    // Our script mod
                    (HMODULE)g_pScripts, 
                    MAKEINTRESOURCEA(ZScriptImportTable::Z_SF)
                )));

        // Fetch the script's internal function table and link the running
        // thread both ways: the script's slot is cleared, ours points at it.
        auto* pInternalFunctions = 
            reinterpret_cast<INTERNALSCRIPTFUNCTIONS*>(
                GetProcAddress(
                    (HMODULE)g_pScripts,
                    MAKEINTRESOURCEA(ZScriptImportTable::Z_ISF)
                ));
        pInternalFunctions->RunningThread = nullptr;
        ISF.RunningThread = reinterpret_cast<_ScriptState*>(pInternalFunctions);

        ScriptsPtr =
            reinterpret_cast<SCRIPTCREATOR**>(
                GetProcAddress(
                    (HMODULE)g_pScripts,
                    MAKEINTRESOURCEA(ZScriptImportTable::Z_Scripts)
                ));

        // Copy the engine's ScriptInterfaces table into the DLL's import
        // table, then run each script creator's Initialize() (+0x1C).
        auto* pScriptImports =
            reinterpret_cast<void**>(
                GetProcAddress(
                    (HMODULE)g_pScripts,
                    MAKEINTRESOURCEA(ZScriptImportTable::Z_ScriptImports)
                ));
        for (int i = 0; i < 0x2CC; ++i)
        {
            pScriptImports[i] = ScriptInterfaces[i];
        }

        if (ScriptsPtr)
        {
            // ScriptsPtr[0] is the list head; creators start at index 1 and
            // the list is nullptr-terminated (ScriptsPtr itself stays put).
            SCRIPTCREATOR* pCreator = ScriptsPtr[1];
            for (int i = 1; pCreator; pCreator = ScriptsPtr[++i])
            {
                pCreator->Initialize();
            }
        }        

        g_bScriptLoadResult = true;
        return g_bScriptLoadResult;
    }

    void ScriptEngine::DetachSceneScripts()
    {
        if (lScriptLoadedCount)
        {
            if (!--lScriptLoadedCount)
            {
                ScriptsPtr = nullptr;
                
                FreeLibrary((HMODULE)g_pScripts);
                g_pScripts = nullptr;
            }
        }
    }

    void ScriptEngine::Sleep(float fTime)
    {
        if (g_pZSC)
        {
            g_pZSC->Sleep(fTime);
        }
    }

    void ScriptEngine::SetRunningThread(_ScriptState* pScript)
    {
        // PS2: INTERNALSCRIPTFUNCTIONS ISF — writes into the RunningThread field
        // (mov ecx, ISF / mov [ecx], eax). PC ISF is the same 4-byte struct.
        ISF.RunningThread = pScript;
    }

    void ScriptEngine::RunNoBreak(_ScriptState* pState)
    {        
        auto* pFuncController = pState->m_pVariables->m_pFunctionController;
        if (!pFuncController)
        {
            return;
        }

        pFuncController->m_pEntryPoint(pState);
    }

    void ScriptEngine::ResumeThread(_ScriptState* pScript)
    {
        if (!pScript) return;

        auto* pScheduledScript = reinterpret_cast<ZScheduledScript*>(pScript->m_pThreadInfo);
        if (!pScheduledScript) return;

        pScheduledScript->Sleep(0.0f);
    }

    void ScriptEngine::StopThread(_ScriptState* pScript)
    {
        if (!pScript) return;

        auto* pScheduledScript = reinterpret_cast<ZScheduledScript*>(pScript->m_pThreadInfo);
        if (!pScheduledScript) return;

        pScheduledScript->Sleep(-1.0f);
    }

    void ScriptEngine::TerminateThread(_ScriptState* pScript)
    {
        if (!pScript) return;

        pScript->m_Flags |= ZSF_TERMINATE;
    }

    int ScriptEngine::GetPriority(_ScriptState* pScript)
    {
        if (!pScript) return 0;
        
        auto* pSchedEvent = static_cast<ZScheduledEvent*>(pScript->m_pThreadInfo);
        if (!pSchedEvent) return 0;

        return static_cast<int>(pSchedEvent->GetPriority());
    }

    void ScriptEngine::SetPriority(_ScriptState* pScript, int lPriority)
    {
        if (!pScript) return;

        ZScheduledEvent* pSchedEvent = static_cast<ZScheduledEvent*>(pScript->m_pThreadInfo);
        if (!pSchedEvent) return;

        auto lPrio = lPriority > 15 ? 15 : lPriority;
        pSchedEvent->SetPriority(lPrio);
    }

    void ScriptEngine::DestroyScriptMessages()
    {
        ZASSERT(g_pZScriptC_Messages != nullptr);
        g_pZScriptC_Messages = nullptr;
        g_pZScriptC_Uniques = nullptr;
    }

    uint16_t ScriptEngine::GetRegisterZMessageID(const char* psName)
    {
        if (!g_pZScriptC_Messages)
        {
            return 0u;
        }

        ZScriptC_ZMessage* pCurrentMsg = g_pZScriptC_Messages;
        int nameOffset = 0;

        while (pCurrentMsg)
        {
            int keyIndex = 0;

            while (true)
            {
                char keyChar = pCurrentMsg->m_pKeys[keyIndex];
                if (!keyChar)
                    return 0;

                if (keyChar == psName[nameOffset])
                    break;

                keyIndex++;
            }

            nameOffset++;

            ZScriptC_Indexes* pIndex = &pCurrentMsg->m_pIndexes[keyIndex];

            if (pIndex->m_lUniqueText != 0)
            {
                const char* pUniqueText = g_pZScriptC_Uniques[pIndex->m_lUniqueText - 1];
                int uniqueOffset = 0;

                while (true)
                {
                    char uChar = pUniqueText[uniqueOffset++];
                    if (!uChar)
                        break;

                    char inputChar = psName[nameOffset++];
                    if (inputChar != uChar)
                        return 0;
                }
            }

            if (pIndex->m_lZMsgID != 0 && psName[nameOffset] == '\0')
            {
                return pIndex->m_lZMsgID;
            }

            if (pIndex->m_lNextIndex == 0)
                break;

            pCurrentMsg = &g_pZScriptC_Messages[pIndex->m_lNextIndex];
        }

        return 0;
    }

    ZREF ScriptEngine::FindScriptStateByRef(ZREF rRef, const char* psScriptName)
    {
        ZGEOM* pGeom = ZGEOM::RefToPtr(rRef);
        if (!pGeom || !pGeom->m_pExData->_Events.ChkEvents())
        {
            return 0;
        }

        if (reinterpret_cast<std::intptr_t>(pGeom->m_pExData) == static_cast<std::intptr_t>(-2))
        {
            return 0;
        }

        ZGeomEventListBuffers::ValueRun sRun {};
        pGeom->m_pExData->_Events.InitValueRun(sRun);
        ZREF rEvent = pGeom->m_pExData->_Events.GetValueFromValueRun(sRun);

        // Walk the geom's event list; for each ZScriptC event check its
        // creator chain (m_pParentCreator) for a matching script name.
        while (rEvent != 1)
        {
            ZEventBase* pEvent = ZEventBase::RefToPtr(rEvent);
            if (pEvent && pEvent->EventName() && !strcmp(pEvent->EventName(), "ScriptC"))
            {
                ZScheduledEvent* pSchedEvent = pEvent->m_pScheduleEvent;
                if (pSchedEvent && pSchedEvent->m_pStoredStack)
                {
                    auto* pState = *reinterpret_cast<_ScriptState**>(pSchedEvent->m_pStoredStack);
                    if (pState)
                    {
                        const SCRIPTCREATOR* pCreator = pState->m_pCreator;
                        while (pCreator)
                        {
                            if (strscriptccmp(pCreator->m_pName, psScriptName))
                            {
                                return pEvent->GetRef();
                            }
                            pCreator = pCreator->m_pParentCreator;
                        }
                    }
                }
            }

            pGeom->m_pExData->_Events.NextValueRun(sRun);
            rEvent = pGeom->m_pExData->_Events.GetValueFromValueRun(sRun);
        }

        return 0;
    }

    void ScriptEngine::Free(void* ptr)
    {
        if (g_pMessageAllocator)
        {
            // Belongs to the message-allocator pool when the payload offset
            // from its base is within (0, 0x1000). Block start = payload - 4.
            const auto lOffsetFromBase =
                static_cast<uint32_t>(
                    reinterpret_cast<const char*>(ptr) - reinterpret_cast<const char*>(g_pMessageAllocator));

            if (lOffsetFromBase > 0 && lOffsetFromBase < 0x1000)
            {
                const uint32_t lBlockOffset =
                    static_cast<uint32_t>(
                        reinterpret_cast<const char*>(ptr) - 4 - reinterpret_cast<const char*>(g_pMessageAllocator));
                const uint32_t lBlockSize = *(reinterpret_cast<const uint32_t*>(ptr) - 1);

                g_pMessageAllocator->Free(lBlockOffset, lBlockSize);
                return;
            }
        }

        g_ScriptAllocator.Free(static_cast<char*>(ptr));
    }

    void ScriptEngine::FreeNM(void* ptr)
    {
        g_ScriptAllocator.Free((char*)ptr);
    }
    
    _ScriptState* ScriptEngine::GetAlienScriptState(ZREF rScript)
    {
        if (!rScript) return nullptr;

        auto* pScript = ZEventBuffer::Instance().ConvEventRefToPtr(rScript);
        if (pScript && pScript->m_pScheduleEvent && pScript->m_pScheduleEvent->m_pStoredStack)
        {
            return *reinterpret_cast<_ScriptState**>(pScript->m_pScheduleEvent->m_pStoredStack);
        }

        return nullptr;
    }

    const _FUNCTIONCONTROLLER* ScriptEngine::GetAlienVirtualTableEntry(ZREF rScript, int lEntryOffset)
    {
        if (!rScript) return nullptr;

        auto* pScript = ZEventBuffer::Instance().ConvEventRefToPtr(rScript);
        if (pScript && pScript->m_pScheduleEvent && pScript->m_pScheduleEvent->m_pStoredStack)
        {
            auto* pState = *reinterpret_cast<_ScriptState**>(pScript->m_pScheduleEvent->m_pStoredStack);
            if (pState && pState->m_pFunctionsVirtualTable)
            {
                // lEntryOffset is a raw byte offset into the function table
                // (mov eax, [eax+ecx] in the PC binary — no index scaling).
                return *reinterpret_cast<const _FUNCTIONCONTROLLER* const*>(
                    reinterpret_cast<const char*>(pState->m_pFunctionsVirtualTable) + lEntryOffset);
            }
        }

        return nullptr;
    }

    const _STATECONTROLLER* ScriptEngine::GetForkThread()
    {
        return ScriptEngine::m_SpecialScriptReturnType.m_pForkStateController;
    }

    uint32_t ScriptEngine::GetFreeScriptMemory()
    {
        return g_ScriptAllocator.GetFreeTotal();
    }

    ZREF ScriptEngine::GetRootScriptStateRef()
    {
        return s_CurrentRootScriptCRef;
    }

    void* ScriptEngine::GetScriptBaseAddress()
    {
        return g_pScripts;
    }

    uint32_t ScriptEngine::GetScriptSize()
    {
        // Not applicable on PC; On PS2 need calculate in AttachSceneScripts method
        return 0u;
    }

    // Build-up buffer for the resolved message name (PS2: char[256] getnamebuffer).
    static char s_GetNameBuffer[256];

    // Recursive trie walk: appends keys/unique texts to s_GetNameBuffer while
    // searching for nMsg. Returns true and leaves the full name in the buffer.
    static bool FindZMessageName(uint16_t nMsg, const ZScriptC_ZMessage* pMessages, int lNameOffset)
    {
        if (!pMessages)
        {
            return false;
        }

        const char* pKeys = pMessages->m_pKeys;
        for (int i = 0; ; ++i)
        {
            const char cKey = pKeys[i];
            if (!cKey)
            {
                break;
            }

            s_GetNameBuffer[lNameOffset] = cKey;
            s_GetNameBuffer[lNameOffset + 1] = '\0';

            const ZScriptC_Indexes* pIndex = &pMessages->m_pIndexes[i];
            if (pIndex->m_lUniqueText)
            {
                strcat(s_GetNameBuffer, g_pZScriptC_Uniques[pIndex->m_lUniqueText - 1]);
            }

            if (pIndex->m_lZMsgID == nMsg)
            {
                return true;
            }

            if (pIndex->m_lNextIndex)
            {
                const int lNextOffset = static_cast<int>(strlen(s_GetNameBuffer));
                if (FindZMessageName(nMsg, &g_pZScriptC_Messages[pIndex->m_lNextIndex], lNextOffset))
                {
                    return true;
                }
            }
        }

        return false;
    }

    const char* ScriptEngine::GetZMessageName(ZMSGID rMessageId)
    {
        if (FindZMessageName(rMessageId, g_pZScriptC_Messages, 0))
        {
            return s_GetNameBuffer;
        }

        return "<unknown>";
    }

    void ScriptEngine::InstallScriptMessages(ZScriptC_ZMessage* pMsg, const char** pUniques)
    {
        ZASSERT(g_pZScriptC_Messages == nullptr);
        
        g_pZScriptC_Messages = pMsg;
        g_pZScriptC_Uniques = pUniques;
    }

    void ScriptEngine::NukeAndRestart()
    {
        lScriptLoadedCount = 0;
    }

    void ScriptEngine::SendCommand(ZREF rSender, ZMSGID Msg, void* pData, ZREF rTarget)
    {
        auto* pSender = ZGEOM::RefToPtr(rSender);
        auto* pTarget = ZGEOM::RefToPtr(rTarget);

        if (pSender)
        {
            pSender->SendCommand(Msg, pData, pTarget);
        }
    }
    
    int ScriptEngine::SendScriptCommand(ZREF rGeomTarget, ZMSGID Msg, void* pData, int rGeomSender)
    {
        if (!rGeomTarget)
        {
            return 0;
        }

        ZGEOM* pReceiver = ZGEOM::RefToPtr(rGeomTarget);
        if (!pReceiver)
        {
            printf("[ScriptEngine::SendScriptCommand(%d, %x, %p, %d)] Trying to send event to null pointer.. probably an import which is not set up correctly!\n", 
                rGeomTarget, Msg, pData, rGeomSender);
            return 0;
        }

        // Sender ref is resolved on PC/PS2 but its result is unused.
        std::ignore = ZGEOM::RefToPtr(rGeomSender);

        if (reinterpret_cast<std::intptr_t>(pReceiver->m_pExData) == static_cast<std::intptr_t>(-2))
        {
            return 0;
        }

        ZGeomEventListBuffers::ValueRun sRun {};
        pReceiver->m_pExData->_Events.InitValueRun(sRun);
        ZREF rEvent = pReceiver->m_pExData->_Events.GetValueFromValueRun(sRun);

        while (rEvent != 1)
        {
            ZEventBase* pEvent = ZEventBase::RefToPtr(rEvent);
            if (pEvent && pEvent->EventName() && !strcmp(pEvent->EventName(), "ScriptC"))
            {
                pEvent->Command(Msg, pData);
            }

            pReceiver->m_pExData->_Events.NextValueRun(sRun);
            rEvent = pReceiver->m_pExData->_Events.GetValueFromValueRun(sRun);
        }

        return 0;
    }

    void ScriptEngine::SetForkStateController(const _STATECONTROLLER* pController)
    {
        ScriptEngine::m_SpecialScriptReturnType.m_pForkStateController = pController;
    }
}