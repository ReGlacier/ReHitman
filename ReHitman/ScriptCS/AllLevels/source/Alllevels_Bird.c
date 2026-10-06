/*
 * Alllevels_Bird.c — ported script functions of the shared bird script
 * "Alllevels_Bird" (PC_Hideout / Hideout.dll).
 *
 *   ENTER 0x1000A6BF  (_Alllevels_Bird_ENTER)
 *
 * The FUNCTIONCONTROLLER / STATECONTROLLER records stay in
 * source/AllLevels.c (parent anchors used by the Hideout_* children — e.g.
 * Hideout_Hideout_Canary_ENTER chains this ENTER).
 *
 * Behaviour: ENTER chains the parent (Alllevels_Baseboid) ENTER through a
 * continuation frame, then completes the bird setup — sets its state flags,
 * applies the boid vision range/angle through
 * Zhm3Boid__Setvisionrangeandangle, and resolves every /Movement/ animation
 * handle used by the (not yet ported) Bird state machine. HeadShot falls back
 * to the first death animation when the model has no dedicated one.
 */

#include <AllLevels/Alllevels_Bird.h>
#include <AllLevels/Alllevels_Baseboid.h>
#include <ScriptRuntime/ScriptSupport.h>

/* ================================================================== */
/* Script variable layout (m_lScriptVariablesSize == 0x78)             */
/* ================================================================== */

/* Mapped only as far as the ported ENTER touches it; the rest of the Bird
 * script (RUN and its states) is not ported and owns the other bytes. */
typedef struct BirdScriptVars
{
    uint8_t  m_ParentReserved[0x0C];  /* +0x00 Alllevels_Baseboid region */
    uint8_t  m_bActive;               /* +0x0C ENTER sets bits 0x02|0x04;
                                       *   bit 0x02 is cleared again when the
                                       *   gate at +0x40 reads 0 */
    uint8_t  m_Padding0D[3];          /* +0x0D */
    uint32_t m_l10;                   /* +0x10 (ENTER forces 0) */
    float    m_f14;                   /* +0x14 (ENTER writes 380.0) */
    uint8_t  m_Padding18[0x10];       /* +0x18 */
    float    m_fVisionRange;          /* +0x28 ENTER: fed to
                                       *   Zhm3Boid__Setvisionrangeandangle
                                       *   as range * 1.5 with angle 270.0 */
    uint8_t  m_Padding2C[0x10];       /* +0x2C */
    uint32_t m_l3C;                   /* +0x3C (ENTER forces 0) */
    uint32_t m_l40;                   /* +0x40 gates clearing bit 0x02 of +0x0C */
    uint8_t  m_Padding44[0x0C];       /* +0x44 */
    anim     m_hAnimDie01;            /* +0x50 "/Movement/Die_01" */
    anim     m_hAnimDieTumble;        /* +0x52 "/Movement/Die_Tumble" */
    anim     m_hAnimFlyFlap;          /* +0x54 "/Movement/Fly_Flap" */
    anim     m_hAnimFlyGlide;         /* +0x56 "/Movement/Fly_Glide" */
    anim     m_hAnimHeadShot;         /* +0x58 "/Movement/HeadShot" (or Die_01) */
    anim     m_hAnimFlyBeforeLanding; /* +0x5A "/Movement/Fly_Flap_BeforeLanding" */
    anim     m_hAnimRunForward;       /* +0x5C "/Movement/Run_Forward" */
    anim     m_hAnimStandPecking;     /* +0x5E "/Movement/Stand_Pecking" */
    anim     m_hAnimStandRelaxed;     /* +0x60 "/Movement/Stand_Relaxed" */
    anim     m_hAnimStandTense;       /* +0x62 "/Movement/Stand_Tense" */
    anim     m_hAnimWalkForward;      /* +0x64 "/Movement/Walk_Forward" */
    uint8_t  m_Padding66[0x12];       /* +0x66 */
} BirdScriptVars; /* 0x78 == creator m_lScriptVariablesSize */

/* ================================================================== */
/* Script functions                                                   */
/* ================================================================== */

/* ENTER — _Alllevels_Bird_ENTER (0x1000A6BF). First resumes the parent
 * (Alllevels_Baseboid) ENTER, then finalizes the bird instance. */
float Alllevels_Bird_ENTER(ScriptState* pState)
{
    BirdScriptVars* sv = (BirdScriptVars*)pState->m_pScriptVariables;
    LocalVarEntry* frame = AS_GetFrame(pState);
    uint16_t resumeIndex = frame->m_lFunctionIndex & 0x7FFFu;

    if (resumeIndex != 0)
    {
        if (resumeIndex != 1)
        {
            if (resumeIndex == 2)
            {
                anim hWalk;
                float fRange;

                /* Parent ENTER returned — PC statement order kept exactly. */
                hWalk = (anim)ScriptImports.Zlink__Getanim(
                    pState->m_rThis, "/Movement/Walk_Forward");
                sv->m_bActive |= 0x02 | 0x04;
                fRange = sv->m_fVisionRange * 1.5f;
                sv->m_hAnimWalkForward = hWalk;
                sv->m_l3C = 0;
                sv->m_l10 = 0;
                sv->m_f14 = 380.0f;

                ScriptImports.Zhm3Boid__Setvisionrangeandangle(
                    pState->m_rThis, fRange, 270.0f);

                if (!sv->m_l40)
                    sv->m_bActive &= (uint8_t)~0x02u;

                sv->m_hAnimFlyFlap = (anim)ScriptImports.Zlink__Getanim(
                    pState->m_rThis, "/Movement/Fly_Flap");
                sv->m_hAnimFlyGlide = (anim)ScriptImports.Zlink__Getanim(
                    pState->m_rThis, "/Movement/Fly_Glide");
                sv->m_hAnimFlyBeforeLanding = (anim)ScriptImports.Zlink__Getanim(
                    pState->m_rThis, "/Movement/Fly_Flap_BeforeLanding");
                sv->m_hAnimHeadShot = (anim)ScriptImports.Zlink__Getanim(
                    pState->m_rThis, "/Movement/HeadShot");
                sv->m_hAnimDie01 = (anim)ScriptImports.Zlink__Getanim(
                    pState->m_rThis, "/Movement/Die_01");
                sv->m_hAnimDieTumble = (anim)ScriptImports.Zlink__Getanim(
                    pState->m_rThis, "/Movement/Die_Tumble");
                sv->m_hAnimRunForward = (anim)ScriptImports.Zlink__Getanim(
                    pState->m_rThis, "/Movement/Run_Forward");
                sv->m_hAnimStandPecking = (anim)ScriptImports.Zlink__Getanim(
                    pState->m_rThis, "/Movement/Stand_Pecking");
                sv->m_hAnimStandRelaxed = (anim)ScriptImports.Zlink__Getanim(
                    pState->m_rThis, "/Movement/Stand_Relaxed");
                sv->m_hAnimStandTense = (anim)ScriptImports.Zlink__Getanim(
                    pState->m_rThis, "/Movement/Stand_Tense");

                if (!sv->m_hAnimHeadShot)          /* no dedicated headshot:
                                                    * fall back to the death anim */
                    sv->m_hAnimHeadShot = sv->m_hAnimDie01;
            }
            SF.Free(frame->m_pNextVariables);
            frame->m_pNextVariables = NULL;
            return AS_ExitCall(pState);
        }
    }
    else
    {
        frame->m_pNextVariables = (LocalVarEntry*)SF.Alloc(0x14, __FILE__, __LINE__);
        if (!frame->m_pNextVariables)
        {
            frame->m_lFunctionIndex = 0;
            return 0.0f;
        }
        frame->m_lNextVariablesSize = 0x14;
        frame->m_lExitFunctionIndex = AS_EXIT_INDEX_NONE;
    }

    AS_InstallCall(pState, frame, 2, &Alllevels_Baseboid_EnterFUNCTIONCONTROLLER);
    return SC_RET_CONTINUE_AFTER_TIMEOUT;
}
