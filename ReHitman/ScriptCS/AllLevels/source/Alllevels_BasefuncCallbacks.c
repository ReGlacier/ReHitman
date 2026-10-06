/*
 * Alllevels_Basefunc callbacks reconstructed from HBMScripts_PC.
 *
 * This file is intentionally disjoint from the existing creator/FC metadata.
 * The owner of that metadata must wire these callbacks after the missing
 * Basefunc FC records and the 0x10045B24 ScriptImports ABI are resolved.
 */

#include <AllLevels/Alllevels_Basefunc.h>
#include <AllLevels/Alllevels_Perceptionconverter.h>
#include <ScriptRuntime/ScriptSupport.h>
#include <ScriptRuntime/ZScriptImportTable.h>
#include <stdint.h>

/* PC 0x1003E7D4: FUNCTIONCONTROLLER { 0x10006DEF, 0x14, 0, ... }.
 * The entry point is not present in this repository; do not substitute a
 * guessed function body or layout. */
extern const FUNCTIONCONTROLLER Alllevels_Basefunc_EnterFUNCTIONCONTROLLER_1003E7D4;

/* Callback entry points for the metadata owner. */
float Alllevels_Basefunc_ENTER_CALLBACK(ScriptState* pState);
void Alllevels_Basefunc_PROCESSMESSAGE_CALLBACK(ScriptState* pState,
                                                uint16_t msgId,
                                                void* pMsgArg);

static void Basefunc_SetDword(uint8_t* sv, uint32_t offset, int32_t value)
{
    *(int32_t*)(sv + offset) = value;
}

/* _Alllevels_Basefunc_ENTER, PC 0x10002BAF.
 * Raw offsets and resume indices are preserved from the compiled script. */
float Alllevels_Basefunc_ENTER_CALLBACK(ScriptState* pState)
{
    LocalVarEntry* frame = AS_GetFrame(pState);
    uint8_t* sv = (uint8_t*)pState->m_pScriptVariables;
    uint16_t resume = frame->m_lFunctionIndex & 0x7FFFu;

    if (resume == 0)
    {
        frame->m_pNextVariables = (LocalVarEntry*)SF.Alloc(24, __FILE__, __LINE__);
        if (!frame->m_pNextVariables)
        {
            frame->m_lFunctionIndex = 0;
            pState->m_Flags &= (uint16_t)~ZSC_CONTINUE_AFTER_SLEEP_MASK;
            return 0.0f;
        }
        frame->m_lNextVariablesSize = 24;
        frame->m_lExitFunctionIndex = AS_EXIT_INDEX_NONE;
    }
    else if (resume == 1)
    {
        sv[0xEC] = 1;
        Basefunc_SetDword(sv, 0x104, 0);
        Basefunc_SetDword(sv, 0x10C, 1128792064);
        Basefunc_SetDword(sv, 0x110, 1063675494);
        *(float*)(sv + 0x114) = 0.0f;
        Basefunc_SetDword(sv, 0x118, (int32_t)0xC33C0000u);
        Basefunc_SetDword(sv, 0x130, 0);
        Basefunc_SetDword(sv, 0x134, 0);
        Basefunc_SetDword(sv, 0x140, 0);
        Basefunc_SetDword(sv, 0x144, 0);
        *(uint16_t*)(sv + 0xED) = (uint16_t)((sv[0xED] & 0xC0u) | 0x0Fu);

        /* PC dword_10045B94(rThis, sv+0x148), then installs the FC at
         * 0x1003E7D4 and resumes at coroutine index 3. */
        AS_InstallCall(pState, frame, 3,
                       &Alllevels_Basefunc_EnterFUNCTIONCONTROLLER_1003E7D4);
        return SC_RET_CONTINUE_AFTER_TIMEOUT;
    }
    else if (resume == 2)
    {
        /* PC tests dword_10045BA4(rThis), optionally calls
         * dword_10045B8C(rThis, name), then calls dword_10045BDC(rThis, 1).
         * The current typed table maps these addresses to incompatible
         * signatures, so this import sequence is blocked until the table
         * baseline/port is corrected. */
    }

    if (resume == 0 || resume == 2 || resume == 3 || resume > 3)
    {
        if (frame->m_pNextVariables)
        {
            SF.Free(frame->m_pNextVariables);
            frame->m_pNextVariables = NULL;
        }
        return AS_ExitCall(pState);
    }

    return AS_ExitCall(pState);
}

/* Verified direct PROCESSMESSAGE effects from PC 0x1000A1FF.  Cases whose
 * bodies allocate/install unreversed FCs are deliberately not represented by
 * guessed stubs; they are listed in the blocker notes below. */
void Alllevels_Basefunc_PROCESSMESSAGE_CALLBACK(ScriptState* pState,
                                                uint16_t msgId,
                                                void* pMsgArg)
{
    uint8_t* sv = (uint8_t*)pState->m_pScriptVariables;
    (void)pMsgArg;

    switch (msgId)
    {
    case 0x0840:
    case 0x0842:
        return;
    case 0x083C:
        sv[0] &= (uint8_t)~8u;
        Basefunc_SetDword(sv, 0x44, 1);
        return;
    case 0x083D:
        sv[0] &= (uint8_t)~8u;
        Basefunc_SetDword(sv, 0x44, 2);
        return;
    case 0x083E:
        sv[0] &= (uint8_t)~8u;
        Basefunc_SetDword(sv, 0x44, 3);
        return;
    case 0x0925:
        sv[0] |= 4u;
        return;
    case 0x0926:
        sv[0] &= (uint8_t)~4u;
        return;
    case 0x0932:
        sv[0xEC] &= (uint8_t)~0x80u;
        return;
    case 0x081D:
        /* PC calls dword_10045EB4(rThis); no typed ScriptImports member has
         * been verified for this address in the current table. */
        return;
    default:
        AS_CallParentProcessMessage(pState, msgId, pMsgArg);
        return;
    }
}

/* Exact blockers for the remaining PC handler calls:
 *
 *   0x0803 -> sub_10009DDD         (direct flag mutation, target offsets need
 *                                   the missing Basefunc script-variable map)
 *   0x0845 -> sub_10009E09         (direct flag mutation, same missing map)
 *   0x0844 -> sub_10009D69         (allocates/switches an unreversed FC)
 *   0x0924 -> sub_10009FD5         (switches FC at PC 0x1003EFBC)
 *   0x092A -> sub_1000A00A         (imports 0x100464A8/0x100464A0)
 *   0x092B -> sub_1000A025         (allocates FC at PC 0x10043710)
 *   0x092C -> sub_1000A095         (allocates FC at PC 0x10043710)
 *   0x092D -> sub_10009E3F         (allocates FC at PC 0x1003EF54)
 *   0x092E -> sub_10009E75         (switches state through DoSwitchState)
 *   0x0933 -> sub_1000A1CA         (switches FC at PC 0x1003EB6C)
 *   0x0946 -> sub_1000A100         (copies 0x28 bytes of message/frame data)
 *
 * Implementing these here would require inventing the missing FC records,
 * message payload layouts, or import signatures, which is explicitly avoided.
 */
