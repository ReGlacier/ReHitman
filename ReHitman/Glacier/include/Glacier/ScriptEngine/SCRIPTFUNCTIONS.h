#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/GlacierFWD.h>
#include <Glacier/ScriptEngine/Fwd.h>
#include <cstdint>


namespace Glacier
{
    #if 0
    static void* Alloc(uint32_t lSize, const char* psFile, uint32_t lLine);
    static void* AllocNM(uint32_t lSize, const char* psFile, uint32_t lLine);
    static uint32_t AllocSize(void* ptr);
    #endif

    struct _SCRIPTFUNCTIONS
    {
        void (*SetForkStateController)();
        ScriptState* (*GetForkThread)();
        void (*Sleep)(float fTime);
        void (*StopThread)(_ScriptState* pScript);
        void (*TerminateThread)(_ScriptState* pScript);
        void (*ResumeThread)(_ScriptState* pScript);
        bool (*CheckTimeout)();
        void (*SendCommand)(ZREF rGeomTarget, ZMSGID Msg, void* pData, int unused);
        void (*SendScriptCommand)(ZREF rGeomTarget, ZMSGID Msg, void* pData, int unused);
        void (*DebugPrint)(const char* format, ...);
        void (*Pack)(void* pData, uint32_t lSize);
        void (*Unpack)(void* pData, uint32_t lSize);
        void (*Input)(void* pData, uint32_t lSize);
        void (*GetZDefine)(const char* pName, void* pData, uint32_t lSize);
        void* (*Alloc)(uint32_t lSize, const char* psFile, uint32_t lLine);
        void* (*AllocNM)(uint32_t lSize, const char* psFile, uint32_t lLine);
        void (*Free)(void* ptr, const char* psFile, uint32_t lLine);
        void (*FreeNM)(void* ptr, const char* psFile, uint32_t lLine);
        void (*RunNoBreak)(_ScriptState* pScript);
        _ScriptState* (*FindScriptStateByRef)(ZREF rRef, const char* pScriptName);
        void* (*GetAlienVirtualTableEntry)(ZREF rRef, int32_t lEntryNr);
        void* (*GetAlienScriptState)(ZREF rRef);
        ZREF (*GetRootScriptStateRef)();
        void (*Memcpy)(void* dst, void* src, int lSize);
        void (*Memset)(void* dst, uint8_t b, int lSize);
        int32_t (*GetPriority)(_ScriptState* pScript);
        void (*SetPriority)(_ScriptState* pScript, int32_t lPriority);
        // In debug PS2 there are 2 more functions
        // void* FunctionCallback;
        // void* SetPriority;
    };
    RE_VERIFY_SIZE(_SCRIPTFUNCTIONS, 0x68);

    using SCRIPTFUNCTIONS = _SCRIPTFUNCTIONS;

    typedef struct _SCRIPTFUNCTIONS SCRIPTFUNCTIONS;
}