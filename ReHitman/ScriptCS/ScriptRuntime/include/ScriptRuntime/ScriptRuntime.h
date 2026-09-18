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
#include "ZScriptImportTable.h"

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

typedef uint16_t ZMSGID;

typedef int  (*ProcessMessage_t)(ZMSGID, void*);
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
    void (*SetForkStateController)(const STATECONTROLLER* pController);            /* +0x00 */
    const STATECONTROLLER* (*GetForkThread)();                                     /* +0x04 */
    void (*Sleep)(float fTime);                                                    /* +0x08 */
    void (*StopThread)(void* pScriptState);                                        /* +0x0C */
    void (*TerminateThread)(void* pScriptState);                                   /* +0x10 */
    void (*ResumeThread)(void* pScriptState);                                      /* +0x14 */
    int (*CheckTimeout)();                                                         /* +0x18 */
    void (*SendCommand)(ZREF rSender, ZMSGID Msg, void* pData, ZREF rTarget);      /* +0x1C */
    int (*SendScriptCommand)(ZREF rGeomTarget, ZMSGID Msg, void* pData, int unused); /* +0x20 */
    void (*DebugPrint)(const char* format, ...);                                   /* +0x24 */
    void (*Pack)(void* pData, uint32_t lSize);                                     /* +0x28 */
    void (*Unpack)(void* pData, uint32_t lSize);                                   /* +0x2C */
    void (*Input)(void* pData, uint32_t lSize);                                    /* +0x30 */
    void (*GetZDefine)(const char* pName, void* pData, uint32_t lSize);            /* +0x34 */
    void* (*Alloc)(uint32_t lSize, const char* psFile, uint32_t lLine);            /* +0x38 */
    void* (*AllocNM)(uint32_t lSize, const char* psFile, uint32_t lLine);          /* +0x3C */
    void (*Free)(void* ptr);                                                       /* +0x40 */
    void (*FreeNM)(void* ptr);                                                     /* +0x44 */
    void (*RunNoBreak)(void* pScriptState);                                        /* +0x48 */
    ZREF (*FindScriptStateByRef)(ZREF rRef, const char* psScriptName);             /* +0x4C */
    const FUNCTIONCONTROLLER* (*GetAlienVirtualTableEntry)(ZREF rRef, int32_t lEntryNr); /* +0x50 */
    void* (*GetAlienScriptState)(ZREF rRef);                                       /* +0x54 */
    ZREF (*GetRootScriptStateRef)();                                               /* +0x58 */
    void (*Memcpy)(void* dst, void* src, uint32_t lSize);                          /* +0x5C */
    void (*Memset)(void* dst, uint8_t b, uint32_t lSize);                          /* +0x60 */
    int32_t (*GetPriority)(void* pScriptState);                                    /* +0x64 */
    void (*SetPriority)(void* pScriptState, int32_t lPriority);                    /* +0x68 */
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
