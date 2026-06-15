#pragma once

#include <Glacier/EventBase/ZEventBase.h>
#include <Glacier/Geom/ZGEOM.h>
#include <cstdint>

namespace Hitman::BloodMoney
{
    struct STATECONTROLLER;

    using ProcessMessage_t = int (*)(unsigned short, void *);
    using VoidFunction_t = void (*)();
    using ScriptFunction_t = void (*)();

    enum SAVEGAMESTATICSTYPE : uint8_t
    {
        SGST_RAW = 0x0,
        SGST_STRING = 0x1,
        SGST_THREAD = 0x2,
        SGST_STATEREF = 0x3,
        SGST_EVENTREF = 0x4,
        SGST_END = 0x5,
    };

    struct SAVEGAMESTATICS
    {
        SAVEGAMESTATICSTYPE m_eType;
        uint8_t m_padding[3];
        uint32_t m_lSize : 24;
        void *m_pAddr;
    };

    struct FUNCTIONCONTROLLER
    {
        ScriptFunction_t m_pEntryPoint;
        int16_t m_lInputSize;
        int16_t m_lDataSize;
        int32_t *m_lStringOffsets;
    };

    struct SCRIPTIMPORT
    {
        uint16_t m_SIT : 3;
        uint16_t m_lAmount : 13;
    };

    struct SCRIPTCREATOR
    {
        char *m_pName;
        int32_t m_lScriptVariablesSize;
        int32_t m_lStateVariablesSize;
        const STATECONTROLLER *m_pStateController;
        const SCRIPTCREATOR *m_pParentCreator;
        const void **m_pStatesVirtualTable;
        ProcessMessage_t ProcessMessage;
        VoidFunction_t Initialize;
        const SAVEGAMESTATICS *m_pSaveGameStatics;
        void (*Imports)();
        void (*StaticImports)();
        void (*UnpackResources)();
        void (*UnpackStaticResources)();
        const SCRIPTIMPORT *m_pImports;
    };

    struct MessageCue
    {
        MessageCue *m_pLast;
        MessageCue *m_pNext;
        uint16_t msg;
    };

    struct LocalVarEntry
    {
        const FUNCTIONCONTROLLER *m_pFunctionController;
        LocalVarEntry *m_pNextVariables;
        LocalVarEntry *m_pPrevVariables;
        int16_t m_lFunctionIndex;
        int16_t m_lExitFunctionIndex;
        int16_t m_lNextVariablesSize;
        int16_t m_lAlignment;
    };

    struct AsyncCall_Struct // sizeof=0xC
    {
        AsyncCall_Struct *m_pNext;
        float m_fStoredNextRun;
        LocalVarEntry *m_pLVE;
    };

    struct ScriptState // sizeof=0x40
    {                  // XREF: ScriptState/r
        void *m_pScriptVariables;
        const SCRIPTCREATOR *m_pCreator;
        Glacier::ZGEOM *m_rThis;
        LocalVarEntry *m_pVariables;
        void *m_pStateVariables;
        const STATECONTROLLER *m_pStateController;
        const STATECONTROLLER *m_pPreviousStateController;
        const STATECONTROLLER *m_pNextStateController;
        uint16_t m_msgWaitForEvent;
        uint16_t m_Flags;
        AsyncCall_Struct *m_pAsyncCall;
        AsyncCall_Struct *m_pAsyncCallLast;
        const SCRIPTCREATOR *m_pMessageHandler;
        ScriptState *m_pAlienCall;
        const void **m_pFunctionsVirtualTable;
        void *m_pThreadInfo;
        MessageCue *m_pMessageCue;
    };

    struct ScriptStateInfo // sizeof=0x8
    {
        ScriptState *m_pRootScriptState;
        ScriptState *m_pCurrentScriptState;
    };

    class ZScriptC : public Glacier::ZEventBase
    {
    public:
        static constexpr const char* Name = "ScriptC";

        // vtbl unchanged

        // methods
        SCRIPTCREATOR* FindScript(const char* scriptName);
        SCRIPTCREATOR* CreateScript(SCRIPTCREATOR* pScriptCreator);

    public: // data
        SCRIPTCREATOR *m_pScriptCreator;
        ScriptStateInfo *m_pInitialScriptStateInfo;
        void *m_pStoredDataBlock;
        int m_msgEnterCamera;
    };

    static_assert(offsetof(ZScriptC, m_pScriptCreator) == 0x30, "Invalid offset of m_pScriptCreator (expected to be 0x30)");
}