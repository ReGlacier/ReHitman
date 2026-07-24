#include <Glacier/ScriptEngine/ScriptEngine.h>
#include <Glacier/ScriptEngine/FUNCTIONCONTROLLER.h>
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
}