#pragma once

#include <Glacier/GlacierFWD.h>
#include <Glacier/ScriptEngine/SpecialScriptReturnType.h>
#include <Glacier/ZUniMemory.h>
#include <cstdint>


namespace Glacier
{
    // fwds
    struct _ScriptState;
    struct _STATECONTROLLER;
    struct _FUNCTIONCONTROLLER;
    struct ZScriptC_ZMessage;

    struct ScriptEngine
    {
        STATIC_CLASS_VAR(ScriptEngine, _SpecialScriptReturnType, m_SpecialScriptReturnType);

        static void* Alloc(uint32_t lSize, const char* psFile, uint32_t lLine);
        static void* AllocNM(uint32_t lSize, const char* psFile, uint32_t lLine);
        static uint32_t AllocSize(void* ptr);
        static bool AttachSceneScripts(const char* pszSceneName);
        static void DestroyScriptMessages();
        static void DetachSceneScripts();
        static ZREF FindScriptStateByRef(ZREF rRef, const char* psScriptName);
        static void Free(void* ptr);
        static void FreeNM(void* ptr);
        static void Sleep(float fTime);
        static void SetPriority(_ScriptState* pScript, int lPriority);
        static void SetRunningThread(_ScriptState* pScript);
        static void RunNoBreak(_ScriptState* pScript);
        static void ResumeThread(_ScriptState* pScript);
        static void StopThread(_ScriptState* pScript);
        static void TerminateThread(_ScriptState* pScript);
        static _ScriptState* GetAlienScriptState(ZREF rScript);
        static const _FUNCTIONCONTROLLER* GetAlienVirtualTableEntry(ZREF rScript, int lEntryOffset);
        static const _STATECONTROLLER* GetForkThread();
        static uint32_t GetFreeScriptMemory();
        static uint16_t GetRegisterZMessageID(const char* psName);
        static ZREF GetRootScriptStateRef();
        static void* GetScriptBaseAddress();
        static uint32_t GetScriptSize();
        static const char* GetZMessageName(ZMSGID rMessageId);
        static void InstallScriptMessages(ZScriptC_ZMessage* pMsg, const char** pUniques);
        static void NukeAndRestart();
        static void SendCommand(ZREF rSender, ZMSGID Msg, void* pData, ZREF rTarget);
        static int SendScriptCommand(ZREF rGeomTarget, ZMSGID Msg, void* pData, int rGeomSender);
        static void SetForkStateController(const _STATECONTROLLER* pController);
        static int GetPriority(_ScriptState* pScript);
        static bool IsScriptCode(const void* ptr);
        static uint32_t GetOffsetInScriptCode(const void* ptr);
        static void* GetAddressInScriptCode(uint32_t rRef);
    };
}