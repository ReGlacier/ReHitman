#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ScriptEngine/Fwd.h>
#include <cstdint>


namespace Glacier
{
    // flags for _ScriptState::m_Flags
    static constexpr uint16_t ZSF_TERMINATE = 0x80u;

    struct _ScriptState
    {
        void* m_pScriptVariables;
        const SCRIPTCREATOR* m_pCreator;
        ZREF m_rThis;
        LocalVarEntry* m_pVariables;
        void* m_pStateVariables;
        const STATECONTROLLER* m_pStateController;
        const STATECONTROLLER* m_pPreviousStateController;
        const STATECONTROLLER* m_pNextStateController;
        ZMSGID m_msgWaitForEvent;
        uint16_t m_Flags;
        AsyncCall_Struct* m_pAsyncCall;
        AsyncCall_Struct* m_pAsyncCallLast;
        const SCRIPTCREATOR* m_pMessageHandler;
        ScriptState* m_pAlienCall;
        const void* m_pFunctionsVirtualTable;
        void* m_pThreadInfo; // related to ZScheduledScript?
        MessageCue* m_pMessageCue;
    };
    RE_VERIFY_SIZE(_ScriptState, 0x40);

    using ScriptState = _ScriptState;
}