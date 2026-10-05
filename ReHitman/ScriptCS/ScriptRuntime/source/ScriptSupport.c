/*
 * ScriptSupport.c — C port of the PC_Hideout "support section" (0x1003D000..)
 * that every compiled ScriptCS DLL embeds. Each function maps 1:1 onto a now
 * renamed PC_Hideout function:
 *
 *   AS_GetFrame                 AS_GetActiveFrame (sub_1003D27C)
 *   AS_InstallCall              AS_InstallCall (sub_1003D299)
 *   AS_ExitCall                 AS_ExitCall (sub_1003D216)
 *   AS_InstallSwitchState       InstallSwitchState (0x1003D8C5)
 *   AS_InstallForcedAsyncCall   InstallForcedAsyncCall (0x1003D460)
 *   AS_UnlinkAsyncCall          AS_UnlinkAsyncCall (sub_1003D188)
 *   AS_CountStateTransitions    AS_CountStateTransitions (sub_1003D06F)
 *   AS_ResizeFrameVars          AS_ResizeFrameVars (sub_1003D0E2)
 *   AS_SwitchState              AS_SwitchState (sub_1003D581)
 *   AS_SwitchStateDriver        AS_SwitchStateDriver (sub_1003D64C)
 *   AS_CallParentProcessMessage the shared creator-chain tail of every
 *                               _PROCESSMESSAGE (pattern at 0x1003C716)
 *
 * Struct field accesses follow the frozen engine layout (ScriptState.h,
 * LocalVarEntry.h, STATECONTROLLER.h, SwitchStateStruct.h). SF /
 * ScriptImports live in a module's Scripts.c (extern here).
 *
 * Frame model (verified against decompiles):
 *   LocalVarEntry+0x04 -> deeper continuation frame, +0x08 -> calling frame,
 *   +0x0C m_lFunctionIndex (resume case; 0x8000 = exiting),
 *   +0x0E m_lExitFunctionIndex (0x7FFF = none, 0xFFFF = switch driver),
 *   locals live at (char*)lve + 0x14.
 */

#include <ScriptRuntime/ScriptSupport.h>

void AS_EmptyVoid(void)
{
    /* nullsub_1 (0x1003C93C) — shared empty Initialize/Imports/Unpack body. */
}

float AS_EmptyEntryPoint(ScriptState* pState)
{
    /* loc_1000721C — shared empty state-enter body. */
    (void)pState;
    return 0.0f;
}

LocalVarEntry* AS_GetFrame(ScriptState* pState)
{
    /* AS_GetActiveFrame: inside a message or outside async use the normal
     * frame chain; while an async call owns the thread use its LVE. */
    uint16_t flags = pState->m_Flags;
    if ((flags & ZSC_FLAG_HANDLING_MESSAGE) != 0 || (flags & ZSC_FLAG_ASYNC_ACTIVE) == 0)
        return pState->m_pVariables;
    return pState->m_pAsyncCall->m_pLVE;
}

void AS_InstallCall(ScriptState* pState, LocalVarEntry* pFrame,
                    uint16_t nextCase, const FUNCTIONCONTROLLER* pCalleeFC)
{
    uint16_t flags = pState->m_Flags;

    if ((flags & ZSC_FLAG_HANDLING_MESSAGE) != 0)
    {
        /* Message context: run the callee inline via a temporary top frame. */
        LocalVarEntry* origTop = pState->m_pVariables;
        LocalVarEntry* frame = pFrame->m_pNextVariables;
        if ((int16_t)pFrame->m_lFunctionIndex >= 0)
            pFrame->m_lFunctionIndex = nextCase;
        if (!frame)
            frame = pFrame;
        frame->m_pFunctionController = pCalleeFC;
        frame->m_pNextVariables = NULL;
        frame->m_pPrevVariables = NULL;
        frame->m_lFunctionIndex = 0;
        frame->m_lExitFunctionIndex = AS_EXIT_INDEX_NONE;
        pState->m_pVariables = frame;
        SF.RunNoBreak(pState);
        pState->m_pVariables = origTop;
    }
    else
    {
        LocalVarEntry* newFrame = pFrame->m_pNextVariables;
        pFrame->m_lFunctionIndex = nextCase;
        newFrame->m_pFunctionController = pCalleeFC;
        newFrame->m_pNextVariables = NULL;
        newFrame->m_pPrevVariables = pFrame;
        newFrame->m_lFunctionIndex = 0;
        newFrame->m_lExitFunctionIndex = AS_EXIT_INDEX_NONE;
        if ((flags & ZSC_FLAG_ASYNC_ACTIVE) != 0)
            pState->m_pAsyncCall->m_pLVE = newFrame;
        else
            pState->m_pVariables = newFrame;
        SF.Memset((char*)newFrame + pCalleeFC->m_lInputSize, 0, pCalleeFC->m_lDataSize);
    }
}

float AS_ExitCall(ScriptState* pState)
{
    uint16_t flags = pState->m_Flags;

    if ((flags & ZSC_FLAG_HANDLING_MESSAGE) != 0 || (flags & ZSC_FLAG_ASYNC_ACTIVE) == 0)
    {
        uint16_t keep = (uint16_t)(flags & ~1u);
        pState->m_Flags = keep;
        if ((keep & ZSC_FLAG_HANDLING_MESSAGE) != 0)
        {
            pState->m_pVariables->m_pFunctionController = NULL;
        }
        else
        {
            LocalVarEntry* frame = pState->m_pVariables;
            LocalVarEntry* caller = frame->m_pPrevVariables;
            if (caller)
                pState->m_pVariables = caller;
            else if ((keep & ZSC_FLAG_ALIEN_CALL_ACTIVE) != 0)
                frame->m_pFunctionController = NULL;
        }
    }
    else
    {
        AsyncCall_Struct* node = pState->m_pAsyncCall;
        LocalVarEntry* caller = node->m_pLVE->m_pPrevVariables;
        if (caller)
            node->m_pLVE = caller;
        else
            AS_UnlinkAsyncCall(pState, node);
    }
    return SC_RET_CONTINUE_AFTER_TIMEOUT;
}

void AS_UnlinkAsyncCall(ScriptState* pState, AsyncCall_Struct* pNode)
{
    AsyncCall_Struct* walk = pState->m_pAsyncCall;
    if (walk == pNode)
    {
        pState->m_pAsyncCall = pNode->pNext;
    }
    else
    {
        while (walk->pNext != pNode)
            walk = walk->pNext;
        walk->pNext = pNode->pNext;
    }
    if (pState->m_pAsyncCallLast == pNode)
        pState->m_pAsyncCallLast = walk;
    SF.Free(pNode->m_pLVE);
    SF.Free(pNode);
}

void AS_InstallForcedAsyncCall(ScriptState* pState, LocalVarEntry* pFrame,
                               uint16_t nextCase, const FUNCTIONCONTROLLER* pFC)
{
    AsyncCall_Struct* chain = pState->m_pAsyncCall;
    int depth = 0;

    if (pFC == &AS_SwitchState_FUNCTIONCONTROLLER && chain != NULL)
    {
        while (chain->m_pLVE->m_pFunctionController != &AS_SwitchStateDriver_FUNCTIONCONTROLLER)
        {
            chain = chain->pNext;
            ++depth;
            if (!chain)
            {
                if (depth > 16)
                    return;
                goto INSTALL;
            }
        }
        return;
    }

INSTALL:
    if (!pFC)
        return;
    pFrame->m_lFunctionIndex = nextCase;
    {
        AsyncCall_Struct* node = (AsyncCall_Struct*)SF.AllocNM(sizeof(AsyncCall_Struct), __FILE__, __LINE__);
        if (!node)
            return;
        {
            LocalVarEntry* lve = (LocalVarEntry*)SF.AllocNM(pFC->m_lInputSize, __FILE__, __LINE__);
            node->m_pLVE = lve;
            if (!lve)
            {
                SF.FreeNM(node);
                return;
            }
            if (pFrame->m_pNextVariables)
                pFrame = pFrame->m_pNextVariables;
            SF.Memcpy((char*)lve, (char*)pFrame, pFC->m_lInputSize - pFC->m_lDataSize);
            SF.Memset((char*)lve + pFC->m_lInputSize - pFC->m_lDataSize, 0, pFC->m_lDataSize);
            lve->m_pFunctionController = pFC;
            lve->m_lFunctionIndex = 0;
            lve->m_pNextVariables = NULL;
            lve->m_pPrevVariables = NULL;

            chain = pState->m_pAsyncCall;
            if (chain)
            {
                if (pFC != &AS_SwitchState_FUNCTIONCONTROLLER)
                {
                    pState->m_pAsyncCallLast->pNext = node;
                    pState->m_pAsyncCallLast = node;
                    node->pNext = NULL;
                    return;
                }
                node->pNext = chain;
            }
            else
            {
                pState->m_pAsyncCallLast = node;
                node->pNext = NULL;
            }
            pState->m_pAsyncCall = node;
        }
    }
}

void AS_CountStateTransitions(const STATECONTROLLER* pFrom, const STATECONTROLLER* pTo,
                              uint16_t* pNumEnters, uint16_t* pNumExits)
{
    const STATECONTROLLER* from = pFrom;
    const STATECONTROLLER* to = pTo;
    uint16_t fromLevel = from ? from->m_lLevel : 0;
    uint16_t toLevel = to->m_lLevel;

    *pNumEnters = 0;
    *pNumExits = 0;
    while (toLevel != fromLevel)
    {
        if (toLevel <= fromLevel)
        {
            from = from->m_pParent;
            --fromLevel;
            ++*pNumExits;
        }
        else
        {
            to = to->m_pParent;
            --toLevel;
            ++*pNumEnters;
        }
    }
    while (fromLevel > 1 && to != from)
    {
        to = to->m_pParent;
        from = from->m_pParent;
        --fromLevel;
        ++*pNumEnters;
        ++*pNumExits;
    }
    if (*pNumEnters == 0 && *pNumExits == 0)
    {
        *pNumEnters = 1;
        ++*pNumExits;
    }
}

int AS_ResizeFrameVars(ScriptState* pState, uint16_t* pFrameVarsSize, uint16_t newSize)
{
    void* newBlock = SF.Alloc(newSize, __FILE__, __LINE__);
    if (newBlock)
    {
        LocalVarEntry* top = pState->m_pVariables;
        void* old = top->m_pNextVariables;
        if (old)
        {
            SF.Memcpy(newBlock, old, *pFrameVarsSize);
            SF.Free(old);
        }
        top->m_pNextVariables = newBlock;
        *pFrameVarsSize = newSize;
        return 1;
    }
    return 0;
}

uint16_t AS_InstallSwitchState(ScriptState* pState, LocalVarEntry* pFrame,
                               uint16_t nextCase, const FUNCTIONCONTROLLER* pSwitchFC)
{
    uint16_t flags = pState->m_Flags;
    void* block;

    if ((flags & ZSC_FLAG_SKIP_MESSAGE_QUEUE) != 0
        && ((flags & ZSC_FLAG_HANDLING_MESSAGE) != 0 || (flags & ZSC_FLAG_ASYNC_WAITING) == 0))
    {
        pFrame->m_lFunctionIndex = nextCase;
        return nextCase;
    }
    if ((flags & ZSC_FLAG_ALIEN_CALL_ACTIVE) != 0)
    {
        if ((flags & ZSC_FLAG_HANDLING_MESSAGE) == 0)
        {
            pFrame->m_lFunctionIndex = nextCase;
            return nextCase;
        }
        block = pFrame->m_pNextVariables ? pFrame->m_pNextVariables : (LocalVarEntry*)pFrame;
    }
    else if ((flags & ZSC_FLAG_HANDLING_MESSAGE) != 0)
    {
        block = pFrame->m_pNextVariables ? pFrame->m_pNextVariables : (LocalVarEntry*)pFrame;
    }
    else
    {
        block = pFrame->m_pNextVariables;
    }

    ((SwitchStateStruct*)block)->m_pEnters = NULL;
    ((SwitchStateStruct*)block)->pSS_pOldStateController = NULL;
    pState->m_Flags = (uint16_t)(flags | ZSC_FLAG_SKIP_MESSAGE_QUEUE | ZSC_FLAG_CLEAR_AFTER_ENTRY);

    if (pState->m_Flags & (ZSC_FLAG_ASYNC_ACTIVE | ZSC_FLAG_HANDLING_MESSAGE))
    {
        ScriptState* t;
        LocalVarEntry* fr;
        AS_InstallForcedAsyncCall(pState, pFrame, nextCase, pSwitchFC);
        while ((t = pState->m_pAlienCall) != NULL)
        {
            for (fr = t->m_pVariables; fr; fr = fr->m_pPrevVariables)
            {
                if (fr->m_lFunctionIndex < fr->m_lExitFunctionIndex)
                    fr->m_lFunctionIndex = fr->m_lExitFunctionIndex;
            }
        }
        return nextCase;
    }
    AS_InstallCall(pState, pFrame, nextCase, pSwitchFC);
    return nextCase;
}

float AS_SwitchState(ScriptState* pState)
{
    /* AS_SwitchState: builds one switch-driver frame below the root frame. */
    LocalVarEntry* chain = pState->m_pVariables;
    SwitchStateStruct* driver;
    LocalVarEntry* active;

    pState->m_Flags &= (uint16_t)~ZSC_CONTINUE_AFTER_SLEEP_MASK;
    driver = (SwitchStateStruct*)SF.Alloc(sizeof(SwitchStateStruct), __FILE__, __LINE__);
    if (!driver)
        return 0.0f;
    pState->m_Flags |= ZSC_FLAG_ASYNC_WAITING;
    active = AS_GetFrame(pState);

    while (chain)
    {
        if (chain->m_lFunctionIndex < chain->m_lExitFunctionIndex)
            chain->m_lFunctionIndex = chain->m_lExitFunctionIndex;
        if (chain->m_pPrevVariables == NULL)
        {
            driver->stateController = ((SwitchStateStruct*)active)->stateController;
            if ((LocalVarEntry*)driver != active)
                pState->m_pNextStateController = driver->stateController;

            driver->m_LVE.m_pNextVariables = chain->m_pNextVariables;
            if (driver->m_LVE.m_pNextVariables)
                driver->m_LVE.m_pNextVariables->m_pPrevVariables = &driver->m_LVE;
            driver->m_LVE.m_pPrevVariables = chain;
            chain->m_pNextVariables = &driver->m_LVE;
            driver->m_LVE.m_lExitFunctionIndex = 0xFFFF;
            driver->m_LVE.m_lFunctionIndex = 0;
            driver->m_LVE.m_pFunctionController = &AS_SwitchStateDriver_FUNCTIONCONTROLLER;

            if (pState->m_pVariables->m_pPrevVariables == NULL)
                pState->m_pVariables = &driver->m_LVE;
        }
        chain = chain->m_pPrevVariables;
    }

    if ((pState->m_Flags & ZSC_FLAG_HANDLING_MESSAGE) != 0
        || (pState->m_Flags & ZSC_FLAG_ASYNC_ACTIVE) == 0)
    {
        pState->m_pVariables = pState->m_pVariables->m_pPrevVariables;
        return SC_RET_CONTINUE_AFTER_TIMEOUT;
    }
    {
        AsyncCall_Struct* node;
        for (node = pState->m_pAsyncCall; node; node = node->pNext)
            node->m_fStoredNextRun = 0.0f;
    }
    return AS_ExitCall(pState);
}

float AS_SwitchStateDriver(ScriptState* pState)
{
    /* AS_SwitchStateDriver: enter/exit chain driver for a state switch. */
    SwitchStateStruct* sw = (SwitchStateStruct*)pState->m_pVariables;
    const STATECONTROLLER* newSC;
    const STATECONTROLLER* targetSC;
    const STATECONTROLLER* parentSC;
    uint16_t frameSize;

    switch (sw->m_LVE.m_lFunctionIndex)
    {
    case 0:
        AS_CountStateTransitions(pState->m_pStateController, sw->stateController,
                                 &sw->m_lNumEnters, &sw->m_lNumExits);
        /* PC falls through into the case-1 builder (LABEL_16). */
        /* fall through */
    case 1:
        if (sw->m_lNumEnters == 0)
            goto LABEL_29;
        sw->m_pEnters = (const STATECONTROLLER**)SF.Alloc(
            (uint32_t)sw->m_lNumEnters * 4u, __FILE__, __LINE__);
        if (!sw->m_pEnters)
        {
            sw->m_LVE.m_lFunctionIndex = 1;
            return 0.0f;
        }
        targetSC = sw->stateController;
        frameSize = sizeof(LocalVarEntry);
        {
            uint16_t i;
            for (i = 0; i < sw->m_lNumEnters; ++i)
            {
                sw->m_pEnters[i] = targetSC;
                if (targetSC->m_pEnter && targetSC->m_pEnter->m_lInputSize > frameSize)
                    frameSize = targetSC->m_pEnter->m_lInputSize;
                targetSC = targetSC->m_pParent;
            }
        }
        sw->m_LVE.m_lNextVariablesSize = frameSize;
        sw->pSS_pOldStateController = pState->m_pStateController;
        goto LABEL_25;
    LABEL_25:
        if (sw->m_LVE.m_pNextVariables)
            SF.Free(sw->m_LVE.m_pNextVariables);
        sw->m_LVE.m_pNextVariables =
            (LocalVarEntry*)SF.Alloc(sw->m_LVE.m_lNextVariablesSize, __FILE__, __LINE__);
        if (sw->m_LVE.m_pNextVariables)
            goto LABEL_33;
        sw->m_LVE.m_lFunctionIndex = 2;
        return 0.0f;
    case 2:
        goto LABEL_25;
    case 3:
    LABEL_29:
        if (sw->stateController)
            goto LABEL_33;
        return SC_RET_TERMINATE;
    case 4:
        goto LABEL_33;
    case 5:
    {
        AsyncCall_Struct* node;
        --sw->m_lNumExits;
        pState->m_pStateController = pState->m_pStateController->m_pParent;
        for (node = pState->m_pAsyncCall; node; node = node->pNext)
        {
            if (node->m_pLVE->m_pFunctionController == &AS_SwitchState_FUNCTIONCONTROLLER)
            {
                pState->m_pPreviousStateController = sw->pSS_pOldStateController;
                pState->m_pNextStateController = NULL;
                goto LABEL_39;
            }
        }
        while (1)
        {
        LABEL_33:
            if (sw->m_lNumExits == 0)
            {
                pState->m_pPreviousStateController = sw->pSS_pOldStateController;
                pState->m_pNextStateController = NULL;
            LABEL_35:
                while (sw->m_lNumEnters != 0)
                {
                    const STATECONTROLLER* enterTarget = sw->m_pEnters[sw->m_lNumEnters - 1];
                    --sw->m_lNumEnters;
                    pState->m_pStateController = enterTarget;
                    if (enterTarget->m_pEnter)
                    {
                        AS_InstallCall(pState, &sw->m_LVE, 0x0B, enterTarget->m_pEnter);
                        return SC_RET_CONTINUE_AFTER_TIMEOUT;
                    }
                }
                sw->m_LVE.m_lFunctionIndex = AS_EXIT_INDEX_NONE;
                goto LABEL_39;
            }
            if (pState->m_pStateController->m_pDestroy)
                break;
            --sw->m_lNumExits;
            pState->m_pStateController = pState->m_pStateController->m_pParent;
        }
        {
            uint16_t destroyInputSize = pState->m_pStateController->m_pDestroy->m_lInputSize;
            if (sw->m_LVE.m_pNextVariables)
            {
                if (destroyInputSize > sw->m_LVE.m_lNextVariablesSize)
                    AS_ResizeFrameVars(pState, &sw->m_LVE.m_lNextVariablesSize, destroyInputSize);
            }
            else
            {
                sw->m_LVE.m_lNextVariablesSize = destroyInputSize;
                sw->m_LVE.m_pNextVariables =
                    (LocalVarEntry*)SF.Alloc(destroyInputSize, __FILE__, __LINE__);
                if (!sw->m_LVE.m_pNextVariables)
                {
                    sw->m_LVE.m_lFunctionIndex = 4;
                    return 0.0f;
                }
            }
        }
        AS_InstallCall(pState, &sw->m_LVE, 5, pState->m_pStateController->m_pDestroy);
        break;
    }
    case 0x0B:
        goto LABEL_35;
    default:
    LABEL_39:
        newSC = sw->stateController;
        if (sw->m_pEnters)
            SF.Free((void*)sw->m_pEnters);
        if (sw->m_LVE.m_pNextVariables)
            SF.Free(sw->m_LVE.m_pNextVariables);
        sw->m_pEnters = NULL;
        sw->m_LVE.m_pNextVariables = NULL;

        pState->m_pStateController = newSC;
        parentSC = pState->m_pCreator->m_pStateController;
        pState->m_pFunctionsVirtualTable =
            (newSC->m_lScriptLevel < parentSC->m_lScriptLevel)
                ? parentSC->m_pFunctionsVirtualTable
                : newSC->m_pFunctionsVirtualTable;

        {
            LocalVarEntry* caller = sw->m_LVE.m_pPrevVariables;
            pState->m_pVariables = caller;
            caller->m_pNextVariables = NULL;
            SF.Free(sw);
            if (pState->m_pVariables->m_pFunctionController != &AS_SwitchStateDriver_FUNCTIONCONTROLLER)
            {
                pState->m_Flags &= (uint16_t)~(ZSC_FLAG_ASYNC_WAITING | ZSC_FLAG_SKIP_MESSAGE_QUEUE);
                pState->m_pVariables->m_pFunctionController = NULL;
            }
        }
        break;
    }
    return SC_RET_CONTINUE_AFTER_TIMEOUT;
}

void AS_CallParentProcessMessage(ScriptState* pState, uint16_t msgId, void* pMsgArg)
{
    const SCRIPTCREATOR* creator = pState->m_pMessageHandler;
    if (creator)
    {
        const SCRIPTCREATOR* parent;
        for (;;)
        {
            parent = creator->m_pParentCreator;
            pState->m_pMessageHandler = parent;
            if (creator->ProcessMessage)
                break;
            if (!parent)
                return;
            creator = parent;
        }
        ((StateProcessMessage_t)creator->ProcessMessage)(pState, msgId, pMsgArg);
    }
}

/* Shared support FC records (PC: 0x10044520 / 0x1004452C). The PC "name"
 * slots point into an unreversed string block; emit NULL (matches the
 * nameless FC records the script functions use). */
const FUNCTIONCONTROLLER AS_SwitchState_FUNCTIONCONTROLLER =
{
    (EntryPoint_t)AS_SwitchState,
    (uint16_t)sizeof(SwitchStateStruct),
    0x00,
    NULL,
    NULL
};

const FUNCTIONCONTROLLER AS_SwitchStateDriver_FUNCTIONCONTROLLER =
{
    (EntryPoint_t)AS_SwitchStateDriver,
    (uint16_t)sizeof(SwitchStateStruct),
    0x00,
    NULL,
    NULL
};
