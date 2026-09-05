#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ScriptEngine/Fwd.h>
#include <cstdint>


namespace Glacier
{
    struct _SCRIPTFUNCTIONS
    {
        // Field order matches the PC binary (InitializeScriptFunctions at 0x0043BE50).
        void (*SetForkStateController)(const _STATECONTROLLER* pController);      // 0x00
        const _STATECONTROLLER* (*GetForkThread)();                               // 0x04
        void (*Sleep)(float fTime);                                               // 0x08
        void (*StopThread)(_ScriptState* pScript);                                // 0x0C
        void (*TerminateThread)(_ScriptState* pScript);                           // 0x10
        void (*ResumeThread)(_ScriptState* pScript);                              // 0x14
        bool (*CheckTimeout)();                                                   // 0x18
        void (*SendCommand)(ZREF rSender, ZMSGID Msg, void* pData, ZREF rTarget);// 0x1C
        int (*SendScriptCommand)(ZREF rGeomTarget, ZMSGID Msg, void* pData, int unused); // 0x20
        void (*DebugPrint)(const char* format, ...);                              // 0x24
        void (*Pack)(void* pData, uint32_t lSize);                                // 0x28
        void (*Unpack)(void* pData, uint32_t lSize);                              // 0x2C
        void (*Input)(void* pData, uint32_t lSize);                               // 0x30
        void (*GetZDefine)(const char* pName, void* pData, uint32_t lSize);       // 0x34
        void* (*Alloc)(uint32_t lSize, const char* psFile, uint32_t lLine);       // 0x38
        void* (*AllocNM)(uint32_t lSize, const char* psFile, uint32_t lLine);     // 0x3C
        void (*Free)(void* ptr);                                                  // 0x40
        void (*FreeNM)(void* ptr);                                                // 0x44
        // PC signatures carry debug call-site info (kept PS2-style for now):
        // void (*Free)(void* ptr, const char* psFile, uint32_t lLine);
        // void (*FreeNM)(void* ptr, const char* psFile, uint32_t lLine);
        void (*RunNoBreak)(_ScriptState* pScript);                                // 0x48
        ZREF (*FindScriptStateByRef)(ZREF rRef, const char* psScriptName);        // 0x4C
        const _FUNCTIONCONTROLLER* (*GetAlienVirtualTableEntry)(ZREF rRef, int32_t lEntryNr);          // 0x50
        _ScriptState* (*GetAlienScriptState)(ZREF rRef);                                  // 0x54
        ZREF (*GetRootScriptStateRef)();                                          // 0x58
        void (*Memcpy)(void* dst, void* src, uint32_t lSize);                     // 0x5C
        void (*Memset)(void* dst, uint8_t b, uint32_t lSize);                     // 0x60
        int32_t (*GetPriority)(_ScriptState* pScript);                            // 0x64
        void (*SetPriority)(_ScriptState* pScript, int32_t lPriority);            // 0x68
    };
    RE_VERIFY_SIZE(_SCRIPTFUNCTIONS, 0x6C);

    using SCRIPTFUNCTIONS = _SCRIPTFUNCTIONS;

    typedef struct _SCRIPTFUNCTIONS SCRIPTFUNCTIONS;
}