#include <Glacier/ScriptEngine/ScriptEngine.h>
#include <Glacier/ScriptEngine/FUNCTIONCONTROLLER.h>
#include <Glacier/ScriptEngine/ZScriptC_ZMessage.h>
#include <Glacier/ScriptEngine/LocalVarEntry.h>
#include <Glacier/ScriptEngine/ScriptState.h>
#include <Glacier/ScriptEngine/Globals.h>
#include <Glacier/EventBase/ZScheduledScript.h>
#include <Windows.h>


namespace Glacier
{
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
        /*
        NOTE: Check later

        in original asm been
        mov eax, [esp + pScript]
        mov ecx, ISF
        mov [ecx], eax
        retn
        */
        ISF = pScript;
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
}