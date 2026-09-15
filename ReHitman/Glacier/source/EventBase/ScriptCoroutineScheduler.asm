.586
.MODEL FLAT, C

; Functions
EXTERN ScriptDeallocate : PROC

; Globals
EXTERN g_pSchedulerSavedESPPtr : DWORD
EXTERN g_pStoredStackPtr       : DWORD
EXTERN g_lStoredStackLengthPtr : DWORD
EXTERN g_pNewStackFramePtr     : DWORD
EXTERN g_pFunctionPtr          : DWORD

.CODE

EnterSchedulerMode PROC NEAR
    ; Save all registers on stack
    pushad

    lea     eax, loc_ResumePoint
    push    eax    ; store ret addr
    mov     eax, g_pSchedulerSavedESPPtr
    mov     [eax], esp

    mov     eax, g_pStoredStackPtr
    cmp     DWORD PTR [eax], 0
    jz      loc_SkipRestore  ; Nothing to restore - skip

    ; Allocate space on stack
    mov     eax, g_lStoredStackLengthPtr
    sub     esp, DWORD PTR [eax]
    mov     eax, g_pNewStackFramePtr
    mov     [eax], esp
    mov     ebp, esp
    push    ebp  ; store frame pointer

    ; Copy previously stored stack on current
    mov     eax, g_lStoredStackLengthPtr
    mov     ecx, DWORD PTR [eax]
    mov     eax, g_pStoredStackPtr
    mov     esi, DWORD PTR [eax]
    mov     eax, g_pNewStackFramePtr
    mov     edi, DWORD PTR [eax]
    
    mov     eax, ecx
    shr     ecx, 2
    rep movsd
    
    mov     ecx, eax
    and     ecx, 3
    rep movsb

    ; Free memory
    mov     eax, g_pStoredStackPtr
    mov     ecx, DWORD PTR [eax]
    mov     [ebp - 4], ecx
    mov     edx, [ebp - 4]
    
    push    edx
    call    ScriptDeallocate
    add     esp, 4

    ; Update globals
    mov     eax, g_pStoredStackPtr
    mov     DWORD PTR [eax], 0
    mov     eax, g_lStoredStackLengthPtr
    mov     DWORD PTR [eax], 0

    add     esp, 4

loc_ResumePoint:
    popad
    ret

loc_SkipRestore:
    mov     eax, g_pFunctionPtr
    jmp     DWORD PTR [eax]

EnterSchedulerMode ENDP

END
