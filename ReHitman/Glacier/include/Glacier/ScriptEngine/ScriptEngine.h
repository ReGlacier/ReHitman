#pragma once

#include <Glacier/GlacierFWD.h>
#include <cstdint>


namespace Glacier
{
    // fwds
    struct _ScriptState;
    struct _STATECONTROLLER;
    struct ZScriptC_ZMessage;

    struct ScriptEngine
    {
        static void* Alloc(uint32_t lSize, const char* psFile, uint32_t lLine);
        static void* AllocNM(uint32_t lSize, const char* psFile, uint32_t lLine);
        static uint32_t AllocSize(void* ptr);
        static void AttachSceneScripts(const char*);
        static void DestroyScriptMessages();
        static void DetachSceneScripts();
        static ZREF FindScriptStateByRef(const char* psName, int unk);
        static void Free(void*);
        static void FreeNM(void*);
        static void Sleep(float fTime);
        static void SetPriority(_ScriptState* pScript, int lPriority);
        static void SetRunningThread(_ScriptState* pScript);
        static void RunNoBreak(_ScriptState* pScript);
        static void ResumeThread(_ScriptState* pScript);
        static void StopThread(_ScriptState* pScript);
        static void TerminateThread(_ScriptState* pScript);
        static void* GetAlienScriptState(ZREF rScript);
        static void* GetAlienVirtualTableEntry(ZREF rScript, int unused);
        static _STATECONTROLLER* GetForkThread();
        static uint32_t GetFreeScriptMemory();
        static uint16_t GetRegisterZMessageID(const char* psName);
        static ZREF GetRootScriptStateRef();
        static void* GetScriptBaseAddress();
        static uint32_t GetScriptSize();
        static const char* GetZMessageName(ZMSGID rMessageId);
        static void InstallScriptMessages(const ZScriptC_ZMessage* pMsg, const char** pUniques);
        static void NukeAndRestart();
        static int SendCommand(ZREF rGeomTarget, ZMSGID Msg, void* pData, int unused);
        static int SendScriptCommand(ZREF rGeomTarget, ZMSGID Msg, void* pData, int unused);
        static void SetForkStateController(const _STATECONTROLLER* pContolelr);
        static int GetPriority(_ScriptState* pScript);
    };
}