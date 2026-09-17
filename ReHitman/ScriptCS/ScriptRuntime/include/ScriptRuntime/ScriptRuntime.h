#ifndef __HBM_SCRIPT_RUNTIME_H__
#define __HBM_SCRIPT_RUNTIME_H__

/*
 * ScriptRuntime — common script-DLL runtime type descriptions.
 *
 * C-facing mirrors of the Glacier script metadata structures shared by every
 * scene script DLL (AllLevels shared scripts + per-script modules). Field
 * order is frozen (see Glacier/ScriptEngine/*.h). Reconstructed from the
 * PC_Hideout reference binary (Hideout.dll).
 *
 * TODO_PTR is the placeholder for any pointer that still resolves into
 * unreversed script code. It expands to NULL so the C tree stays buildable;
 * `grep -r TODO_PTR` lists exactly what is still missing.
 */

#include <stdint.h>

#ifndef TODO_PTR
#define TODO_PTR NULL
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SCRIPTCREATOR SCRIPTCREATOR;
typedef struct STATECONTROLLER STATECONTROLLER;
typedef struct FUNCTIONCONTROLLER FUNCTIONCONTROLLER;
typedef struct SAVEGAMESTATICS SAVEGAMESTATICS;
typedef struct SCRIPTIMPORT SCRIPTIMPORT;

typedef int  (*ProcessMessage_t)(unsigned short, void*);
typedef void (*VoidFunction_t)(void);
typedef float (*EntryPoint_t)(void* pScriptState); /* float yield protocol */

/* FUNCTIONCONTROLLER — 0x10, one per callable */
struct FUNCTIONCONTROLLER
{
    EntryPoint_t      m_pEntryPoint;      /* +0x00 */
    uint16_t          m_lInputSize;       /* +0x04 */
    uint16_t          m_lDataSize;        /* +0x06 */
    const char*       m_pName;            /* +0x08 */
    uint16_t*         m_lStringOffsets;   /* +0x0C */
};

/* STATECONTROLLER — one per state */
struct STATECONTROLLER
{
    const FUNCTIONCONTROLLER* m_pRun;                 /* +0x00 */
    const FUNCTIONCONTROLLER* m_pEnter;               /* +0x04 */
    const FUNCTIONCONTROLLER* m_pDestroy;             /* +0x08 */
    void (*ProcessMessage)();                         /* +0x0C */
    const void* m_pFunctionsVirtualTable;             /* +0x10 */
    const uint16_t m_lLevel;                          /* +0x14 */
    const uint16_t m_lScriptLevel;                    /* +0x16 */
    const STATECONTROLLER* m_pParent;                 /* +0x18 */
    const char* m_pName;                              /* +0x1C */
    uint16_t* m_lStringOffsets;                       /* +0x20 */
};

/* SAVEGAMESTATICS — 8 bytes */
struct SAVEGAMESTATICS
{
    uint32_t m_eType : 8;   /* +0x00 */
    int32_t  m_lSize  : 24; /* +0x00 */
    void*    m_pAddr;       /* +0x04 */
};

/* SCRIPTIMPORT — 2 bytes */
struct SCRIPTIMPORT
{
    uint16_t m_SIT     : 3;  /* +0x00 */
    uint16_t m_lAmount : 13; /* +0x00 */
};

struct SCRIPTCREATOR
{
    const char*            m_pName;                 /* +0x00 */
    int32_t                m_lScriptVariablesSize;  /* +0x04 */
    int32_t                m_lStateVariablesSize;   /* +0x08 */
    const STATECONTROLLER* m_pStateController;      /* +0x0C */
    const SCRIPTCREATOR*   m_pParentCreator;        /* +0x10 */
    const void*            m_pStatesVirtualTable;   /* +0x14 */
    ProcessMessage_t       ProcessMessage;          /* +0x18 */
    VoidFunction_t         Initialize;              /* +0x1C */
    const SAVEGAMESTATICS* m_pSaveGameStatics;      /* +0x20 */
    VoidFunction_t         Imports;                 /* +0x24 */
    VoidFunction_t         StaticImports;           /* +0x28 */
    VoidFunction_t         UnpackResources;         /* +0x2C */
    VoidFunction_t         UnpackStaticResources;   /* +0x30 */
    const SCRIPTIMPORT*    m_pImports;              /* +0x34 */
};

/* SCRIPTFUNCTIONS — 0x6C (27 pointers), engine → script */
typedef struct SCRIPTFUNCTIONS
{
    void* SetForkStateController;      /* +0x00 */
    void* GetForkThread;               /* +0x04 */
    void* Sleep;                       /* +0x08 */
    void* StopThread;                  /* +0x0C */
    void* TerminateThread;             /* +0x10 */
    void* ResumeThread;                /* +0x14 */
    void* CheckTimeout;                /* +0x18 */
    void* SendCommand;                 /* +0x1C */
    void* SendScriptCommand;           /* +0x20 */
    void* DebugPrint;                  /* +0x24 */
    void* Pack;                        /* +0x28 */
    void* Unpack;                      /* +0x2C */
    void* Input;                       /* +0x30 */
    void* GetZDefine;                  /* +0x34 */
    void* Alloc;                       /* +0x38 */
    void* AllocNM;                     /* +0x3C */
    void* Free;                        /* +0x40 */
    void* FreeNM;                      /* +0x44 */
    void* RunNoBreak;                  /* +0x48 */
    void* FindScriptStateByRef;        /* +0x4C */
    void* GetAlienVirtualTableEntry;   /* +0x50 */
    void* GetAlienScriptState;         /* +0x54 */
    void* GetRootScriptStateRef;       /* +0x58 */
    void* Memcpy;                      /* +0x5C */
    void* Memset;                      /* +0x60 */
    void* GetPriority;                /* +0x64 */
    void* SetPriority;                /* +0x68 */
} SCRIPTFUNCTIONS; /* 0x6C */

/* INTERNALSCRIPTFUNCTIONS — 4 bytes */
typedef struct INTERNALSCRIPTFUNCTIONS
{
    void* RunningThread; /* +0x00 */
} INTERNALSCRIPTFUNCTIONS;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* __HBM_SCRIPT_RUNTIME_H__ */
