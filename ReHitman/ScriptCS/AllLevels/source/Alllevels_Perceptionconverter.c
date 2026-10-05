#include <AllLevels/Alllevels_Perceptionconverter.h>
#include <AllLevels/Alllevels_Levelcontrol.h>
#include <ScriptRuntime/ScriptSupport.h>
#include <string.h>

static uint8_t s_PercStaticA[0x100];
static uint8_t s_PercStaticB[0x100];
static uint8_t s_PercStaticC[0x100];
static uint32_t s_PercStaticD[15];
static int32_t s_PercStaticE;
static int32_t s_PercStaticF;
static int32_t s_PercStaticG;
static float s_PercStaticH[41];
static uint8_t s_PercStaticI[0x100];
static int32_t s_PercStaticJ;
static float s_PercStaticK;
static float s_PercStaticL;

const SAVEGAMESTATICS Alllevels_Perceptionconverter_SaveGameStatics[] =
{
    { 0, 0x100, s_PercStaticA },
    { 0, 0x100, s_PercStaticB },
    { 0, 0x100, s_PercStaticC },
    { 0, 0x3C, s_PercStaticD },
    { 0, 4, &s_PercStaticE },
    { 0, 4, &s_PercStaticF },
    { 0, 4, &s_PercStaticG },
    { 0, 0xA4, s_PercStaticH },
    { 0, 0x100, s_PercStaticI },
    { 0, 4, &s_PercStaticJ },
    { 0, 4, &s_PercStaticK },
    { 0, 4, &s_PercStaticL },
    { 5, 0, NULL }
};

const SCRIPTIMPORT Alllevels_Perceptionconverter_ImportDescriptors[] =
{
    { 0, 1 }, { 0, 1 }, { 6, 0 }, { 0, 0 }
};

static uint16_t s_Perceptionconverter_OffsetStrings[] =
{
    3, 0x20, 0x38, 3, 0xA0, 0xB4, 0, 0,
    3, 0x14, 0x18, 0, 6, 0, 0x24, 0,
    0, 0, 0x3C0, 0
};

float Alllevels_Perceptionconverter_RUN(ScriptState* pState)
{
    LocalVarEntry* frame = AS_GetFrame(pState);
    switch (frame->m_lFunctionIndex)
    {
    case 0:
        pState->m_pFunctionsVirtualTable =
            pState->m_pCreator->m_pStateController->m_pFunctionsVirtualTable;
    case 1:
        frame->m_pNextVariables = (LocalVarEntry*)SF.Alloc(0x18, __FILE__, __LINE__);
        if (!frame->m_pNextVariables)
        {
            frame->m_lFunctionIndex = 1;
            pState->m_Flags &= (uint16_t)~ZSC_CONTINUE_AFTER_SLEEP_MASK;
            return 0.0f;
        }
        frame->m_lExitFunctionIndex = AS_EXIT_INDEX_NONE;
    case 2:
        AS_InstallCall(pState, frame, 3,
                       &Alllevels_Perceptionconverter_EnterFUNCTIONCONTROLLER);
        return SC_RET_CONTINUE_AFTER_TIMEOUT;
    case 3:
        SF.Free(frame->m_pNextVariables);
        frame->m_pNextVariables = NULL;
    case 4:
        frame->m_pNextVariables = (LocalVarEntry*)SF.Alloc(0x24, __FILE__, __LINE__);
        if (!frame->m_pNextVariables)
        {
            frame->m_lFunctionIndex = 4;
            pState->m_Flags &= (uint16_t)~ZSC_CONTINUE_AFTER_SLEEP_MASK;
            return 0.0f;
        }
        pState->m_Flags &= (uint16_t)~(ZSC_FLAG_ASYNC_WAITING | ZSC_FLAG_SKIP_MESSAGE_QUEUE);
        frame->m_lFunctionIndex = 5;
    case 5:
        ((SwitchStateStruct*)frame->m_pNextVariables)->stateController =
            &Alllevels_Perceptionconverter_State_Idle;
        AS_InstallSwitchState(pState, frame, 6, &AS_SwitchState_FUNCTIONCONTROLLER);
        return 0.0f;
    case 6:
        SF.Free(frame->m_pNextVariables);
        frame->m_pNextVariables = NULL;
        return SC_RET_TERMINATE;
    default:
        return SC_RET_TERMINATE;
    }
}

void Alllevels_Perceptionconverter_IMPORTS(ScriptState* pState)
{
    uint8_t* sv = (uint8_t*)pState->m_pScriptVariables;
    uint8_t importedByte;
    SF.Input(&importedByte, 1);
    sv[0] = (uint8_t)((sv[0] & (uint8_t)~1u) | (importedByte & 1u));
    SF.Input(&importedByte, 1);
    sv[0] = (uint8_t)((sv[0] & (uint8_t)~8u) | ((importedByte << 3) & 8u));
}

void Alllevels_Perceptionconverter_INITIALIZE(void)
{
    s_PercStaticK = 0.0f;
    s_PercStaticA[0] = 0;
    s_PercStaticB[0] = 0;
    s_PercStaticC[0] = 0;
    s_PercStaticG = 1;
    s_PercStaticI[0] = 0;
    s_PercStaticJ = 0;
    s_PercStaticL = -1.0f;
}

float Alllevels_Perceptionconverter_ENTER(ScriptState* pState)
{
    LocalVarEntry* frame = AS_GetFrame(pState);
    uint8_t* sv = (uint8_t*)pState->m_pScriptVariables;
    uint16_t resume = frame->m_lFunctionIndex & 0x7FFFu;
    if (resume == 0)
    {
        frame->m_pNextVariables = (LocalVarEntry*)SF.Alloc(0xB8, __FILE__, __LINE__);
        if (!frame->m_pNextVariables)
        {
            frame->m_lFunctionIndex = 0;
            return 0.0f;
        }
        frame->m_lNextVariablesSize = 0xB8;
        frame->m_lExitFunctionIndex = AS_EXIT_INDEX_NONE;
    }
    if (resume <= 1)
    {
        sv[0] = (uint8_t)((sv[0] & 9u) | 0x40u);
        sv[1] = 19;
        memset(sv + 2, 0, 0xEA);
        return AS_ExitCall(pState);
    }
    if (resume == 3 || resume == 4)
    {
        if (SF.CheckTimeout())
            return 0.0f;
    }
    if (resume == 5 || resume == 7)
        return AS_ExitCall(pState);
    return AS_ExitCall(pState);
}

void Alllevels_Perceptionconverter_PROCESSMESSAGE(
    ScriptState* pState, uint16_t msgId, void* pMsgArg)
{
    (void)pState;
    (void)msgId;
    (void)pMsgArg;
    /* TODO: Finish me after PerceptionConverter message imports are reversed. */
}

float Alllevels_Perceptionconverter_State_SetMode1(ScriptState* pState)
{
    LocalVarEntry* frame = AS_GetFrame(pState);
    if ((frame->m_lFunctionIndex & 0x7FFFu) == 0)
        frame->m_lExitFunctionIndex = AS_EXIT_INDEX_NONE;
    return AS_ExitCall(pState);
}

float Alllevels_Perceptionconverter_State_SetMode0(ScriptState* pState)
{
    LocalVarEntry* frame = AS_GetFrame(pState);
    if ((frame->m_lFunctionIndex & 0x7FFFu) == 0)
        frame->m_lExitFunctionIndex = AS_EXIT_INDEX_NONE;
    return AS_ExitCall(pState);
}

const FUNCTIONCONTROLLER Alllevels_Perceptionconverter_RunFUNCTIONCONTROLLER =
{
    (EntryPoint_t)Alllevels_Perceptionconverter_RUN, 0x14, 0, NULL, NULL
};

const FUNCTIONCONTROLLER Alllevels_Perceptionconverter_EnterFUNCTIONCONTROLLER =
{
    (EntryPoint_t)Alllevels_Perceptionconverter_ENTER, 0x18, 4, NULL, NULL
};

const FUNCTIONCONTROLLER Alllevels_Perceptionconverter_FunctionCONTROLLERS[16] =
{
    { TODO_PTR, 0x18, 0, NULL, NULL },
    { TODO_PTR, 0x50, 0x14, NULL, NULL },
    { TODO_PTR, 0x20, 0, NULL, NULL },
    { (EntryPoint_t)AS_EmptyEntryPoint, 0x14, 0, NULL, NULL },
    { (EntryPoint_t)AS_EmptyEntryPoint, 0x14, 0, NULL, NULL },
    { (EntryPoint_t)AS_EmptyEntryPoint, 0x14, 0, NULL, NULL },
    { (EntryPoint_t)AS_EmptyEntryPoint, 0x14, 0, NULL, NULL },
    { (EntryPoint_t)AS_EmptyEntryPoint, 0x14, 0, NULL, NULL },
    { (EntryPoint_t)AS_EmptyEntryPoint, 0x14, 0, NULL, NULL },
    { TODO_PTR, 0x44, 0, NULL, NULL },
    { TODO_PTR, 0x1C, 0, NULL, NULL },
    { TODO_PTR, 0x4C, 0, NULL, NULL },
    { TODO_PTR, 0x68, 0x20, NULL, NULL },
    { TODO_PTR, 0x44, 0, NULL, NULL },
    { TODO_PTR, 0x44, 1, NULL, NULL },
    { TODO_PTR, 0x3C, 0, NULL, NULL }
};

const void* const Alllevels_Perceptionconverter_FunctionsVT[16] =
{
    &Alllevels_Perceptionconverter_FunctionCONTROLLERS[0],
    &Alllevels_Perceptionconverter_FunctionCONTROLLERS[1],
    &Alllevels_Perceptionconverter_FunctionCONTROLLERS[2],
    &Alllevels_Perceptionconverter_FunctionCONTROLLERS[3],
    &Alllevels_Perceptionconverter_FunctionCONTROLLERS[4],
    &Alllevels_Perceptionconverter_FunctionCONTROLLERS[5],
    &Alllevels_Perceptionconverter_FunctionCONTROLLERS[6],
    &Alllevels_Perceptionconverter_FunctionCONTROLLERS[7],
    &Alllevels_Perceptionconverter_FunctionCONTROLLERS[8],
    &Alllevels_Perceptionconverter_FunctionCONTROLLERS[9],
    &Alllevels_Perceptionconverter_FunctionCONTROLLERS[10],
    &Alllevels_Perceptionconverter_FunctionCONTROLLERS[11],
    &Alllevels_Perceptionconverter_FunctionCONTROLLERS[12],
    &Alllevels_Perceptionconverter_FunctionCONTROLLERS[13],
    &Alllevels_Perceptionconverter_FunctionCONTROLLERS[14],
    &Alllevels_Perceptionconverter_FunctionCONTROLLERS[15]
};

const STATECONTROLLER Alllevels_Perceptionconverter_ROOTSTATE =
{
    &Alllevels_Perceptionconverter_RunFUNCTIONCONTROLLER,
    &Alllevels_Perceptionconverter_EnterFUNCTIONCONTROLLER,
    NULL, NULL, Alllevels_Perceptionconverter_FunctionsVT,
    1, 1, NULL, NULL, s_Perceptionconverter_OffsetStrings
};

const STATECONTROLLER Alllevels_Perceptionconverter_State_Idle =
{
    &Alllevels_Levelcontrol_Idle_RunFUNCTIONCONTROLLER,
    NULL, NULL, NULL, Alllevels_Perceptionconverter_FunctionsVT,
    2, 1, &Alllevels_Perceptionconverter_ROOTSTATE, NULL, NULL
};
