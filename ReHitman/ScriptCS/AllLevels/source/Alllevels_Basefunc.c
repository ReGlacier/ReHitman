#include <AllLevels/Alllevels_Basefunc.h>
#include <AllLevels/Alllevels_Perceptionconverter.h>
#include <ScriptRuntime/ScriptSupport.h>
#include <stddef.h>

static uint16_t s_BasefuncOffsets[] = { 5, 0, 0, 0 };

/* Neutral field names denote verified offsets, not inferred semantics. */
#pragma pack(push, 4)
typedef struct BasefuncStatics
{
    int32_t m_Static0;       /* +0x00: PC dword_1004688C */
    int32_t m_Static4;       /* +0x04: PC dword_10046890 */
    int32_t m_Static8;       /* +0x08: PC dword_10046894 */
    int32_t m_Static0C;      /* +0x0C: PC dword_10046898 */
    uint16_t m_Static10;     /* +0x10: PC word_1004689C */
    uint16_t m_Padding12;
    int32_t m_Static14;      /* +0x14: PC dword_100468A0 */
    int32_t m_Static18;      /* +0x18: PC dword_100468A4 */
    int32_t m_Static1C;      /* +0x1C: PC dword_100468A8 */
    float   m_Static20;      /* +0x20: PC flt_100468AC */
    int32_t m_Static24;      /* +0x24: PC dword_100468B0 */
    uint8_t m_Static28;      /* +0x28: PC byte_100468B4 */
    uint8_t m_Padding29[3];  /* +0x29 */
    int32_t m_Static2C;      /* +0x2C: PC dword_100468B8 */
    int32_t m_Static30;      /* +0x30: PC dword_100468BC */
    int32_t m_Static34;      /* +0x34: PC dword_100468C0 */
    int32_t m_Static38;      /* +0x38: PC dword_100468C4 */
    uint16_t m_Static3C;     /* +0x3C: PC word_100468C8 */
    uint16_t m_Padding3E;    /* +0x3E */
    int32_t m_Static40;      /* +0x40: PC dword_100468CC */
    int32_t m_Static44;      /* +0x44: PC dword_100468D0 */
    int32_t m_Static48;      /* +0x48: PC dword_100468D4 */
    int32_t m_Static4C;      /* +0x4C: PC dword_100468D8 */
    float   m_Static50;      /* +0x50: PC flt_100468DC */
    int32_t m_Static54;      /* +0x54: PC dword_100468E0 */
} BasefuncStatics;

typedef struct BasefuncScriptVariables
{
    /* The parent owns this region; Basefunc must not initialize it itself. */
    uint8_t m_ParentReserved[0xEC]; /* +0x00: Perceptionconverter */
    uint8_t m_FlagsEC;       /* +0xEC */
    uint8_t m_FlagsED;       /* +0xED: preserve bits 6/7 on entry */
    uint8_t m_FlagsEE;       /* +0xEE */
    uint8_t m_FlagsEF;       /* +0xEF */
    uint8_t m_PaddingF0[0x14]; /* +0xF0..+0x103 */
    int32_t m_StateValue104;  /* +0x104 */
    uint8_t m_Padding108[4];  /* +0x108 */
    float   m_StateValue10C;  /* +0x10C */
    float   m_StateValue110;  /* +0x110 */
    float   m_StateValue114;  /* +0x114 */
    float   m_StateValue118;  /* +0x118 */
    uint8_t m_Padding11C[0x14]; /* +0x11C..+0x12F */
    int32_t m_StateValue130;  /* +0x130 */
    int32_t m_StateValue134;  /* +0x134 */
    uint8_t m_Reserved138[8]; /* +0x138..+0x13F: not initialized by ENTER */
    int32_t m_StateValue140;  /* +0x140 */
    int32_t m_StateValue144;  /* +0x144 */
    int32_t m_Imported148;    /* +0x148: signed selector in PC ENTER */
    uint8_t m_Padding14C[0x2C]; /* +0x14C..+0x177 */
    uint32_t m_Imported178;   /* +0x178 */
} BasefuncScriptVariables;
#pragma pack(pop)

static BasefuncStatics s_BasefuncStatics;

/* Compile-time ABI checks compatible with the project's MSVC C mode. */
typedef char BasefuncStatics_SizeCheck[(sizeof(BasefuncStatics) == 0x58) ? 1 : -1];
typedef char BasefuncScriptVariables_SizeCheck[(sizeof(BasefuncScriptVariables) == 0x17C) ? 1 : -1];

#define BASEFUNC_CHECK_OFFSET(type, member, offset) \
    typedef char type##_##member##_OffsetCheck[(offsetof(type, member) == offset) ? 1 : -1]
BASEFUNC_CHECK_OFFSET(BasefuncStatics, m_Static0, 0x00);
BASEFUNC_CHECK_OFFSET(BasefuncStatics, m_Static4, 0x04);
BASEFUNC_CHECK_OFFSET(BasefuncStatics, m_Static8, 0x08);
BASEFUNC_CHECK_OFFSET(BasefuncStatics, m_Static0C, 0x0C);
BASEFUNC_CHECK_OFFSET(BasefuncStatics, m_Static10, 0x10);
BASEFUNC_CHECK_OFFSET(BasefuncStatics, m_Static14, 0x14);
BASEFUNC_CHECK_OFFSET(BasefuncStatics, m_Static18, 0x18);
BASEFUNC_CHECK_OFFSET(BasefuncStatics, m_Static1C, 0x1C);
BASEFUNC_CHECK_OFFSET(BasefuncStatics, m_Static20, 0x20);
BASEFUNC_CHECK_OFFSET(BasefuncStatics, m_Static24, 0x24);
BASEFUNC_CHECK_OFFSET(BasefuncStatics, m_Static28, 0x28);
BASEFUNC_CHECK_OFFSET(BasefuncStatics, m_Static2C, 0x2C);
BASEFUNC_CHECK_OFFSET(BasefuncStatics, m_Static30, 0x30);
BASEFUNC_CHECK_OFFSET(BasefuncStatics, m_Static34, 0x34);
BASEFUNC_CHECK_OFFSET(BasefuncStatics, m_Static38, 0x38);
BASEFUNC_CHECK_OFFSET(BasefuncStatics, m_Static3C, 0x3C);
BASEFUNC_CHECK_OFFSET(BasefuncStatics, m_Static40, 0x40);
BASEFUNC_CHECK_OFFSET(BasefuncStatics, m_Static44, 0x44);
BASEFUNC_CHECK_OFFSET(BasefuncStatics, m_Static48, 0x48);
BASEFUNC_CHECK_OFFSET(BasefuncStatics, m_Static4C, 0x4C);
BASEFUNC_CHECK_OFFSET(BasefuncStatics, m_Static50, 0x50);
BASEFUNC_CHECK_OFFSET(BasefuncStatics, m_Static54, 0x54);
BASEFUNC_CHECK_OFFSET(BasefuncScriptVariables, m_FlagsEC, 0xEC);
BASEFUNC_CHECK_OFFSET(BasefuncScriptVariables, m_FlagsED, 0xED);
BASEFUNC_CHECK_OFFSET(BasefuncScriptVariables, m_FlagsEE, 0xEE);
BASEFUNC_CHECK_OFFSET(BasefuncScriptVariables, m_FlagsEF, 0xEF);
BASEFUNC_CHECK_OFFSET(BasefuncScriptVariables, m_StateValue104, 0x104);
BASEFUNC_CHECK_OFFSET(BasefuncScriptVariables, m_StateValue10C, 0x10C);
BASEFUNC_CHECK_OFFSET(BasefuncScriptVariables, m_StateValue110, 0x110);
BASEFUNC_CHECK_OFFSET(BasefuncScriptVariables, m_StateValue114, 0x114);
BASEFUNC_CHECK_OFFSET(BasefuncScriptVariables, m_StateValue118, 0x118);
BASEFUNC_CHECK_OFFSET(BasefuncScriptVariables, m_StateValue130, 0x130);
BASEFUNC_CHECK_OFFSET(BasefuncScriptVariables, m_StateValue134, 0x134);
BASEFUNC_CHECK_OFFSET(BasefuncScriptVariables, m_StateValue140, 0x140);
BASEFUNC_CHECK_OFFSET(BasefuncScriptVariables, m_StateValue144, 0x144);
BASEFUNC_CHECK_OFFSET(BasefuncScriptVariables, m_Imported148, 0x148);
BASEFUNC_CHECK_OFFSET(BasefuncScriptVariables, m_Imported178, 0x178);
#undef BASEFUNC_CHECK_OFFSET

/* Preserve PC registration order and field sizes; padding is not serialized. */
#define BASEFUNC_STATIC(member) \
    { 0, sizeof(s_BasefuncStatics.member), &s_BasefuncStatics.member }
const SAVEGAMESTATICS Alllevels_Basefunc_SaveGameStatics[] =
{
    BASEFUNC_STATIC(m_Static10),
    BASEFUNC_STATIC(m_Static3C),
    BASEFUNC_STATIC(m_Static38),
    BASEFUNC_STATIC(m_Static28),
    BASEFUNC_STATIC(m_Static18),
    BASEFUNC_STATIC(m_Static4C),
    BASEFUNC_STATIC(m_Static40),
    BASEFUNC_STATIC(m_Static0),
    BASEFUNC_STATIC(m_Static48),
    BASEFUNC_STATIC(m_Static14),
    BASEFUNC_STATIC(m_Static2C),
    BASEFUNC_STATIC(m_Static4),
    BASEFUNC_STATIC(m_Static8),
    BASEFUNC_STATIC(m_Static50),
    BASEFUNC_STATIC(m_Static1C),
    BASEFUNC_STATIC(m_Static34),
    BASEFUNC_STATIC(m_Static54),
    BASEFUNC_STATIC(m_Static30),
    BASEFUNC_STATIC(m_Static24),
    BASEFUNC_STATIC(m_Static0C),
    BASEFUNC_STATIC(m_Static44),
    BASEFUNC_STATIC(m_Static20),
    { 5, 0, NULL }
};
#undef BASEFUNC_STATIC

void Alllevels_Basefunc_INITIALIZE(void)
{
    s_BasefuncStatics.m_Static50 = 0.0f;
    s_BasefuncStatics.m_Static10 = 0;
    s_BasefuncStatics.m_Static3C = 0;
    s_BasefuncStatics.m_Static20 = 0.0f;
    s_BasefuncStatics.m_Static28 = 1;
    s_BasefuncStatics.m_Static18 = 2;
    s_BasefuncStatics.m_Static4C = 4;
    s_BasefuncStatics.m_Static40 = 6;
    s_BasefuncStatics.m_Static0 = 7;
    s_BasefuncStatics.m_Static48 = 8;
    s_BasefuncStatics.m_Static14 = 16;
    s_BasefuncStatics.m_Static2C = 13;
    s_BasefuncStatics.m_Static4 = 14;
    s_BasefuncStatics.m_Static8 = 17;
    s_BasefuncStatics.m_Static1C = 0;
    s_BasefuncStatics.m_Static34 = 0;
    s_BasefuncStatics.m_Static54 = 0;
    s_BasefuncStatics.m_Static30 = 0;
    s_BasefuncStatics.m_Static24 = 0;
    s_BasefuncStatics.m_Static0C = 0;
    s_BasefuncStatics.m_Static44 = 0;
}

void Alllevels_Basefunc_IMPORTS(ScriptState* pState)
{
    BasefuncScriptVariables* sv = (BasefuncScriptVariables*)pState->m_pScriptVariables;
    Alllevels_Perceptionconverter_IMPORTS(pState);
    SF.Input(&sv->m_Imported148, sizeof(sv->m_Imported148));
    SF.Input(&sv->m_Imported178, sizeof(sv->m_Imported178));
}

float Alllevels_Basefunc_RUN(ScriptState* pState)
{
    LocalVarEntry* frame = AS_GetFrame(pState);
    switch (frame->m_lFunctionIndex)
    {
    case 0:
        pState->m_pFunctionsVirtualTable =
            pState->m_pCreator->m_pStateController->m_pFunctionsVirtualTable;
        frame->m_pNextVariables = (LocalVarEntry*)SF.Alloc(0x14, __FILE__, __LINE__);
        if (!frame->m_pNextVariables)
        {
            frame->m_lFunctionIndex = 1;
            pState->m_Flags &= (uint16_t)~ZSC_CONTINUE_AFTER_SLEEP_MASK;
            return 0.0f;
        }
        frame->m_lExitFunctionIndex = AS_EXIT_INDEX_NONE;
    case 1:
        AS_InstallCall(pState, frame, 2,
                       &Alllevels_Basefunc_EnterFUNCTIONCONTROLLER);
        return SC_RET_CONTINUE_AFTER_TIMEOUT;
    case 2:
        SF.Free(frame->m_pNextVariables);
        frame->m_pNextVariables = NULL;
        frame->m_pNextVariables = (LocalVarEntry*)SF.Alloc(0x24, __FILE__, __LINE__);
        if (!frame->m_pNextVariables)
        {
            frame->m_lFunctionIndex = 3;
            pState->m_Flags &= (uint16_t)~ZSC_CONTINUE_AFTER_SLEEP_MASK;
            return 0.0f;
        }
        frame->m_lFunctionIndex = 3;
    case 3:
        ((SwitchStateStruct*)frame->m_pNextVariables)->stateController =
            &Alllevels_Basefunc_State_Start;
        AS_InstallSwitchState(pState, frame, 4, &AS_SwitchState_FUNCTIONCONTROLLER);
        return SC_RET_CONTINUE_AFTER_TIMEOUT;
    case 4:
        SF.Free(frame->m_pNextVariables);
        frame->m_pNextVariables = NULL;
        return SC_RET_TERMINATE;
    default:
        return SC_RET_TERMINATE;
    }
}

/* Only the field-initialization portion is ported here. */
// TODO: Finish me after the parent/callee continuation and engine calls are ported.
float Alllevels_Basefunc_ENTER(ScriptState* pState)
{
    LocalVarEntry* frame = AS_GetFrame(pState);
    BasefuncScriptVariables* sv = (BasefuncScriptVariables*)pState->m_pScriptVariables;
    uint16_t resume = frame->m_lFunctionIndex & 0x7FFFu;
    if (resume == 0)
    {
        sv->m_FlagsEC = 1;
        /* PC writes a word at +0xED using only the old low byte. */
        sv->m_FlagsED = (uint8_t)((sv->m_FlagsED & 0xC0u) | 0x0Fu);
        sv->m_FlagsEE = 0;
        sv->m_FlagsEF = 0;
        sv->m_StateValue104 = 0;
        sv->m_StateValue10C = 200.0f;
        sv->m_StateValue110 = 0.9f;
        sv->m_StateValue114 = 0.0f;
        sv->m_StateValue118 = -200.0f;
        sv->m_StateValue130 = 0;
        sv->m_StateValue134 = 0;
        sv->m_StateValue140 = 0;
        sv->m_StateValue144 = 0;
        return AS_ExitCall(pState);
    }
    return AS_ExitCall(pState);
}

float Alllevels_Basefunc_Start_RUN(ScriptState* pState)
{
    LocalVarEntry* frame = AS_GetFrame(pState);
    for (;;)
    {
        uint16_t resume = frame->m_lFunctionIndex & 0x7FFFu;
        if (resume == AS_EXIT_INDEX_NONE)
        {
            frame->m_pFunctionController = NULL;
            return 0.0f;
        }
        if (resume != 0 && resume != 1)
        {
            frame->m_lFunctionIndex = 1;
            if (SF.CheckTimeout())
                return 0.0f;
        }
        else
        {
            if (resume == 0)
                frame->m_lExitFunctionIndex = AS_EXIT_INDEX_NONE;
            frame->m_lFunctionIndex = 2;
            return 100.0f;
        }
    }
}

void Alllevels_Basefunc_PROCESSMESSAGE(ScriptState* pState, uint16_t msgId, void* pMsgArg)
{
    (void)pState;
    (void)msgId;
    (void)pMsgArg;
    /* TODO: Finish me after Basefunc message handlers are ported. */
}

const FUNCTIONCONTROLLER Alllevels_Basefunc_RunFUNCTIONCONTROLLER =
{ (EntryPoint_t)Alllevels_Basefunc_RUN, 0x14, 0, NULL, NULL };
const FUNCTIONCONTROLLER Alllevels_Basefunc_EnterFUNCTIONCONTROLLER =
{ (EntryPoint_t)Alllevels_Basefunc_ENTER, 0x14, 0, NULL, NULL };
const FUNCTIONCONTROLLER Alllevels_Basefunc_Start_RunFUNCTIONCONTROLLER =
{ (EntryPoint_t)Alllevels_Basefunc_Start_RUN, 0x14, 0, NULL, NULL };
const FUNCTIONCONTROLLER Alllevels_Basefunc_Start_EnterFUNCTIONCONTROLLER =
{ (EntryPoint_t)AS_EmptyEntryPoint, 0x14, 0, NULL, NULL };

const FUNCTIONCONTROLLER Alllevels_Basefunc_FunctionCONTROLLERS[1] =
{
    { TODO_PTR, 0, 0, NULL, NULL }
};
const void* const Alllevels_Basefunc_FunctionsVT[1] =
{
    &Alllevels_Basefunc_FunctionCONTROLLERS[0]
};

const STATECONTROLLER Alllevels_Basefunc_ROOTSTATE =
{
    &Alllevels_Basefunc_RunFUNCTIONCONTROLLER,
    &Alllevels_Basefunc_EnterFUNCTIONCONTROLLER,
    NULL, NULL, Alllevels_Basefunc_FunctionsVT,
    1, 2, &Alllevels_Perceptionconverter_ROOTSTATE, NULL,
    s_BasefuncOffsets
};

const STATECONTROLLER Alllevels_Basefunc_State_Start =
{
    &Alllevels_Basefunc_Start_RunFUNCTIONCONTROLLER,
    &Alllevels_Basefunc_Start_EnterFUNCTIONCONTROLLER,
    NULL, NULL, Alllevels_Basefunc_FunctionsVT,
    2, 2, &Alllevels_Basefunc_ROOTSTATE, NULL, NULL
};

const void* const Alllevels_Basefunc_StateVT[7] =
{
    NULL, NULL, NULL, NULL, NULL, NULL, NULL
};
