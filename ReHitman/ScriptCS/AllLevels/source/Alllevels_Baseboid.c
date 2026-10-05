/*
 * Alllevels_Baseboid.c — C port of the shared base-boid script
 * "Alllevels_Baseboid" (PC_Hideout / Hideout.dll). Fully ported:
 *
 *   RUN                        0x1000263F  (_Alllevels_Baseboid_RUN)
 *   STATICINITIALIZERS         0x1000270D  (_Alllevels_Baseboid_STATICINITIALIZERS)
 *   ENTER                      0x10002719  (_Alllevels_Baseboid_ENTER)
 *   PlayAnimWait               0x1000274F  (_Alllevels_Baseboid_Playanimwait)
 *   PlayAnimSegment            0x100027E6  (_Alllevels_Baseboid_Playanimsegment)
 *   PlayAnimSegmentWait        0x10002858  (_Alllevels_Baseboid_Playanimsegmentwait)
 *   PlayAnimInterpolatedWait   0x100028EF  (_Alllevels_Baseboid_Playaniminterpolatedwait)
 *   PROCESSMESSAGE             0x100029D1  (_Alllevels_Baseboid_PROCESSMESSAGE)
 *   Imports/StaticImports/Unpack* = shared empty stub (nullsub_1)
 *
 * Creator 0x1003E5C0 and the FUNCTIONCONTROLLER / STATECONTROLLER anchors
 * live in source/AllLevels.c; entries point at the functions below (see
 * Docs/PC_Hideout_CONTEXT.md §7 for the compiled-ABI conventions).
 *
 * The Baseboid protocol: two animation channels.
 *  - var +0x04 holds the handle of a fire-and-forget Zlink::PlayAnimSegment,
 *  - var +0x08 holds the handle of a wait-style segment (this script's three
 *    Wait/Segment helpers all use it).
 * A message 0x803 carrying one of those handles sets the matching cancel bit
 * (var +0x00 bit 0 for +0x04, bit 1 for +0x08) — the wait helpers clear
 * bit 1 when (re)starting and unwind as soon as it is set again, so the
 * engine can interrupt a running boid animation. Every synchronous segment
 * start is wrapped in state-flag bit 0x0001 (unnamed in ScriptFlags.h).
 */

#include <AllLevels/Alllevels_Baseboid.h>
#include <ScriptRuntime/ScriptSupport.h>

/* ================================================================== */
/* Script variables (m_lScriptVariablesSize == 0x0C) and script statics */
/* ================================================================== */

typedef struct BaseboidScriptVars
{
    uint8_t  m_SegmentFlags;   /* +0x00 bit 0 = cancel for the handle at +0x04,
                                *   bit 1 = cancel for the handle at +0x08;
                                *   set by PROCESSMESSAGE 0x803 / ENTER,
                                *   cleared by the segment helpers on start */
    uint8_t  m_Padding01[3];   /* +0x01 */
    int32_t  m_lSegmentHandle; /* +0x04 handle from PlayAnimSegment (channel A) */
    int32_t  m_lWaitHandle;    /* +0x08 handle from the wait helpers (channel B) */
} BaseboidScriptVars; /* 0x0C == creator m_lScriptVariablesSize */

/* Script static written by STATICINITIALIZERS (PC dword_100468E4, registered
 * in the creator's SAVEGAMESTATICS list as { type 0, size 4 }). */
static ZREF s_rHitman;

/* ================================================================== */
/* Frame locals (frame vars start at lve + 0x14 = the FC input size)   */
/* ================================================================== */

/* PlayAnimSegment / PlayAnimSegmentWait locals (FC input 0x28). PC reads the
 * segment parameters at lve+20/24/28/32 and the animation at lve+36. */
typedef struct BaseboidSegmentParams
{
    int32_t  m_lSegment;   /* +0x00 */
    float    m_fStart;     /* +0x04 */
    float    m_fBlend;     /* +0x08 */
    float    m_fEnd;       /* +0x0C */
    uint16_t m_hAnim;      /* +0x10 (two of the four bytes) */
    uint16_t m_Padding12;  /* +0x12 */
} BaseboidSegmentParams;

/* PlayAnimInterpolatedWait locals (FC input 0x44). */
typedef struct BaseboidInterpolatedParams
{
    int32_t  m_lFlags;     /* +0x00 */
    float    m_fBlendIn;   /* +0x04 */
    float    m_fStart;     /* +0x08 */
    float    m_fBlendOut;  /* +0x0C */
    float    m_fEnd;       /* +0x10 */
    uint16_t m_hAnim;      /* +0x14 */
    uint16_t m_Padding16;  /* +0x16 */
    v3       m_Dir;        /* +0x18 */
    v3       m_Pos;        /* +0x24 */
} BaseboidInterpolatedParams;

/* ================================================================== */
/* Script functions                                                   */
/* ================================================================== */

/* root RUN — _Alllevels_Baseboid_RUN (0x1000263F). The classic root driver:
 * install ENTER, wait, prepare a switch-state block (never filled — children
 * own the states), finish. */
float Alllevels_Baseboid_RUN(ScriptState* pState)
{
    LocalVarEntry* frame = AS_GetFrame(pState);

    switch (frame->m_lFunctionIndex)
    {
    case 0:
        pState->m_pFunctionsVirtualTable =
            pState->m_pCreator->m_pStateController->m_pFunctionsVirtualTable;
        /* fall through */
    case 1:
    {
        void* block = SF.Alloc(0x14, __FILE__, __LINE__);
        frame->m_pNextVariables = (LocalVarEntry*)block;
        if (!block)
        {
            frame->m_lFunctionIndex = 1;
            pState->m_Flags &= (uint16_t)~ZSC_CONTINUE_AFTER_SLEEP_MASK;
            return 0.0f;
        }
        frame->m_lExitFunctionIndex = AS_EXIT_INDEX_NONE;
        /* fall through */
    }
    case 2:
        AS_InstallCall(pState, frame, 3, &Alllevels_Baseboid_EnterFUNCTIONCONTROLLER);
        return SC_RET_CONTINUE_AFTER_TIMEOUT;
    case 3:
        SF.Free(frame->m_pNextVariables);
        frame->m_pNextVariables = NULL;
        /* fall through */
    case 4:
        frame->m_pNextVariables = (LocalVarEntry*)SF.Alloc(0x24, __FILE__, __LINE__);
        if (frame->m_pNextVariables)
        {
            pState->m_Flags &= (uint16_t)~(ZSC_FLAG_ASYNC_WAITING | ZSC_FLAG_SKIP_MESSAGE_QUEUE);
            frame->m_lFunctionIndex = 5;
        }
        else
        {
            frame->m_lFunctionIndex = 4;
            pState->m_Flags &= (uint16_t)~ZSC_CONTINUE_AFTER_SLEEP_MASK;
        }
        return 0.0f;
    case 5:
        SF.Free(frame->m_pNextVariables);
        frame->m_pNextVariables = NULL;
        break;
    default:
        break;
    }
    return SC_RET_TERMINATE;
}

/* ENTER — _Alllevels_Baseboid_ENTER (0x10002719).
 * Root-state enter: requests a cancel on the wait channel and unwinds. */
float Alllevels_Baseboid_ENTER(ScriptState* pState)
{
    LocalVarEntry* frame = AS_GetFrame(pState);
    uint16_t resumeIndex = frame->m_lFunctionIndex & 0x7FFFu;

    if (resumeIndex <= 1)
    {
        if (resumeIndex == 0)
            frame->m_lExitFunctionIndex = AS_EXIT_INDEX_NONE;
        ((BaseboidScriptVars*)pState->m_pScriptVariables)->m_SegmentFlags |= 0x02u;
    }
    return AS_ExitCall(pState);
}

/* PROCESSMESSAGE — _Alllevels_Baseboid_PROCESSMESSAGE (0x100029D1).
 * 0x803 carries an animation-segment handle; whichever channel matches gets
 * its cancel bit set (the wait loops stop on it). Nothing is forwarded to
 * the parent chain (Baseboid is the root script). */
void Alllevels_Baseboid_PROCESSMESSAGE(ScriptState* pState, uint16_t msgId, void* pMsgArg)
{
    BaseboidScriptVars* sv = (BaseboidScriptVars*)pState->m_pScriptVariables;

    if (msgId == 0x803)
    {
        int32_t lHandle = *(const int32_t*)pMsgArg;
        if (sv->m_lSegmentHandle == lHandle)
            sv->m_SegmentFlags |= 0x01u;
        else if (sv->m_lWaitHandle == lHandle)
            sv->m_SegmentFlags |= 0x02u;
    }
}

/* STATICINITIALIZERS — _Alllevels_Baseboid_STATICINITIALIZERS (0x1000270D).
 * Caches the Hitman reference in the script static registered with the save
 * framework (PC stores it in dword_100468E4). */
void Alllevels_Baseboid_INITIALIZE(void)
{
    s_rHitman = ScriptImports.Silevelcontrol__Gethitman();
}

/* PlayAnimWait — _Alllevels_Baseboid_Playanimwait (0x1000274F).
 * FC input 0x18: the animation handle (u16) in the frame's first local,
 * segment 1, start 0.0, end -1.0, blend 1.0. */
float Alllevels_Baseboid_PlayAnimWait(ScriptState* pState)
{
    BaseboidScriptVars* sv = (BaseboidScriptVars*)pState->m_pScriptVariables;
    LocalVarEntry* frame = AS_GetFrame(pState);
    uint16_t resumeIndex = frame->m_lFunctionIndex & 0x7FFFu;

    if (resumeIndex > 2)
        return AS_ExitCall(pState);

    if (resumeIndex == 0)
    {
        pState->m_Flags |= 0x0001u;
        frame->m_lExitFunctionIndex = AS_EXIT_INDEX_NONE;
    }

    if (resumeIndex <= 1)
    {
        /* Start segment 1 of the animation handed over by the caller. */
        sv->m_SegmentFlags &= (uint8_t)~0x02u;
        sv->m_lWaitHandle = ScriptImports.Zlink__Playanimsegment(
            pState->m_rThis,
            *(const uint16_t*)AS_FRAME_VARS(frame),
            1, 0.0f, -1.0f, 1.0f);
        pState->m_Flags &= (uint16_t)~0x0001u;

        if (!sv->m_lWaitHandle)
            return AS_ExitCall(pState);
    }

    /* Wait: leave when the cancel bit was set (PROCESSMESSAGE 0x803 or ENTER)
     * or a message is being handled on this thread. */
    if ((sv->m_SegmentFlags & 0x02u) != 0 || (pState->m_Flags & ZSC_FLAG_HANDLING_MESSAGE) != 0)
        return AS_ExitCall(pState);

    frame->m_lFunctionIndex = 2;
    return SC_RET_RETRY_NEXT_PASS;
}

/* PlayAnimSegment — _Alllevels_Baseboid_Playanimsegment (0x100027E6).
 * Fire-and-forget segment on channel A (var +0x04), no wait loop. */
float Alllevels_Baseboid_PlayAnimSegment(ScriptState* pState)
{
    BaseboidScriptVars* sv = (BaseboidScriptVars*)pState->m_pScriptVariables;
    LocalVarEntry* frame = AS_GetFrame(pState);
    uint16_t resumeIndex = frame->m_lFunctionIndex & 0x7FFFu;

    if (resumeIndex <= 1)
    {
        if (resumeIndex == 0)
        {
            pState->m_Flags |= 0x0001u;
            frame->m_lExitFunctionIndex = AS_EXIT_INDEX_NONE;
        }

        sv->m_SegmentFlags &= (uint8_t)~0x01u;
        sv->m_lSegmentHandle = 0;
        {
            const BaseboidSegmentParams* v =
                (const BaseboidSegmentParams*)AS_FRAME_VARS(frame);
            sv->m_lSegmentHandle = ScriptImports.Zlink__Playanimsegment(
                pState->m_rThis, v->m_hAnim, v->m_lSegment,
                v->m_fStart, v->m_fEnd, v->m_fBlend);
        }
        pState->m_Flags &= (uint16_t)~0x0001u;
    }
    return AS_ExitCall(pState);
}

/* PlayAnimSegmentWait — _Alllevels_Baseboid_Playanimsegmentwait (0x10002858).
 * Same wait loop as PlayAnimWait but with explicit segment parameters
 * (channel B handle). */
float Alllevels_Baseboid_PlayAnimSegmentWait(ScriptState* pState)
{
    BaseboidScriptVars* sv = (BaseboidScriptVars*)pState->m_pScriptVariables;
    LocalVarEntry* frame = AS_GetFrame(pState);
    uint16_t resumeIndex = frame->m_lFunctionIndex & 0x7FFFu;

    if (resumeIndex > 2)
        return AS_ExitCall(pState);

    if (resumeIndex == 0)
    {
        pState->m_Flags |= 0x0001u;
        frame->m_lExitFunctionIndex = AS_EXIT_INDEX_NONE;
    }

    if (resumeIndex <= 1)
    {
        const BaseboidSegmentParams* v = (const BaseboidSegmentParams*)AS_FRAME_VARS(frame);
        sv->m_SegmentFlags &= (uint8_t)~0x02u;
        sv->m_lWaitHandle = 0;
        sv->m_lWaitHandle = ScriptImports.Zlink__Playanimsegment(
            pState->m_rThis, v->m_hAnim, v->m_lSegment, v->m_fStart, v->m_fEnd, v->m_fBlend);
        pState->m_Flags &= (uint16_t)~0x0001u;

        if (!sv->m_lWaitHandle)
            return AS_ExitCall(pState);
    }

    if ((sv->m_SegmentFlags & 0x02u) != 0 || (pState->m_Flags & ZSC_FLAG_HANDLING_MESSAGE) != 0)
        return AS_ExitCall(pState);

    frame->m_lFunctionIndex = 2;
    return SC_RET_RETRY_NEXT_PASS;
}

/* PlayAnimInterpolatedWait — _Alllevels_Baseboid_Playaniminterpolatedwait
 * (0x100028EF). Interpolated segment (position/direction from the frame) with
 * the same cancel-bit wait loop on channel B. */
float Alllevels_Baseboid_PlayAnimInterpolatedWait(ScriptState* pState)
{
    BaseboidScriptVars* sv = (BaseboidScriptVars*)pState->m_pScriptVariables;
    LocalVarEntry* frame = AS_GetFrame(pState);
    uint16_t resumeIndex = frame->m_lFunctionIndex & 0x7FFFu;

    if (resumeIndex > 2)
        return AS_ExitCall(pState);

    if (resumeIndex == 0)
    {
        pState->m_Flags |= 0x0001u;
        frame->m_lExitFunctionIndex = AS_EXIT_INDEX_NONE;
    }

    if (resumeIndex <= 1)
    {
        const BaseboidInterpolatedParams* v =
            (const BaseboidInterpolatedParams*)AS_FRAME_VARS(frame);
        sv->m_SegmentFlags &= (uint8_t)~0x02u;
        sv->m_lWaitHandle = 0;
        sv->m_lWaitHandle = ScriptImports.Zlink__Playaniminterpolated(
            pState->m_rThis, v->m_hAnim, v->m_Pos, v->m_Dir,
            v->m_fStart, v->m_fEnd, v->m_fBlendIn, v->m_fBlendOut, v->m_lFlags);
        pState->m_Flags &= (uint16_t)~0x0001u;

        if (!sv->m_lWaitHandle)
            return AS_ExitCall(pState);
    }

    if ((sv->m_SegmentFlags & 0x02u) != 0 || (pState->m_Flags & ZSC_FLAG_HANDLING_MESSAGE) != 0)
        return AS_ExitCall(pState);

    frame->m_lFunctionIndex = 2;
    return SC_RET_RETRY_NEXT_PASS;
}

/* ================================================================== */
/* Save-game statics (PC 0x1003E5B0: { type 0, size 4, &dword_100468E4 }
 * terminated by { SGST_END })                                           */
/* ================================================================== */

const SAVEGAMESTATICS Alllevels_Baseboid_SaveGameStatics[] =
{
    { 0, 4, &s_rHitman },  /* PC { 0x00000400, 0x100468E4 } — ZREF static */
    { 5, 0, NULL }         /* SGST_END terminator                         */
};
