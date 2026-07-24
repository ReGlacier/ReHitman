#pragma once


namespace Glacier
{
    using ProcessMessage_t = int (*)(unsigned short, void *);
    using VoidFunction_t = void (*)();
    using ScriptFunction_t = void (*)();

    struct _LocalVarEntry;
    using LocalVarEntry = _LocalVarEntry;

    struct _FUNCTIONCONTROLLER;
    using FUNCTIONCONTROLLER = _FUNCTIONCONTROLLER;

    struct _SCRIPTCREATOR;
    using SCRIPTCREATOR = _SCRIPTCREATOR;

    struct _ScriptState;
    using ScriptState = _ScriptState;

    struct _STATECONTROLLER;
    using STATECONTROLLER = _STATECONTROLLER;

    struct _AsyncCall_Struct;
    using AsyncCall_Struct = _AsyncCall_Struct;

    struct _MessageCue;
    using MessageCue = _MessageCue;

    struct _SCRIPTIMPORTS;
    using SCRIPTIMPORT = _SCRIPTIMPORTS;

    struct _SAVEGAMESTATICS;
    using SAVEGAMESTATICS = _SAVEGAMESTATICS;

    struct _SwitchStateStruct;
    using SwitchStateStruct = _SwitchStateStruct;

    struct _INTERNALSCRIPTFUNCTIONS;
    using INTERNALSCRIPTFUNCTIONS = _INTERNALSCRIPTFUNCTIONS;
    
    struct _MsgLocalVarEntry;
    using MsgLocalVarEntry = _MsgLocalVarEntry;

    struct ScriptStateInfo;
}