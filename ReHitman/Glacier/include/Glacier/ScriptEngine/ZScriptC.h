#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ScriptEngine/Fwd.h>
#include <Glacier/CBaseEvent.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/ZSTL/ZRTStringObject.h>
#include <Glacier/ZSTL/TIMETYPE.h>
#include <cstdint>


namespace Glacier
{
    class ZScriptC : public CBaseEvent<ZGEOM>
    {
    public:
        // static
        STATIC_CLASS_VAR(ZScriptC, RTP::ZPropertyInfo, Info);

        // static
        static constexpr const char* Name = "ScriptC";

        // types

        // vtbl
        ~ZScriptC() override;
        void PostSave(ISerializerStream& stream) override;
        bool PostLoad(ISerializerStream& stream) override;
        const RTP::ZPropertyInfo& GetProperties() const override;
        void Init2() override;
        void PostInit() override;
        void PreSaveGame() override;
        void FrameUpdate() override;
        int Command(ZMSGID command, ZDATA data) override;
        void SchedUpdate() override;

        // methods
        ZScriptC();

        ScriptStateInfo* CreateScript(const SCRIPTCREATOR* pCreator);
        static const SCRIPTCREATOR* FindScript(const char* pszScriptName);
        ScriptState* ForkState(const STATECONTROLLER* pController);
        ScriptState* GetRootScriptState();
        bool IsValidThread(ScriptState* pScriptState);
        bool TerminateScript();
        bool FreeThread(ScriptStateInfo* pState);
        int GetNrThreads();
        const ZScheduledScript* GetSchedEvent();
        TIMETYPE GetNextRun();
        int GetPriority();
        void LoadSaveGame(ISerializerStream& stream);
        static void NukeAndRestart();

#       pragma region " --- Runtime --- "
        void GetName(ZRTString& name);
        void SetName(const ZRTString& name);
#       pragma endregion

        // members
        union 
        {
            const _SCRIPTCREATOR* m_pScriptCreator { nullptr };
            ScriptStateInfo* m_pInitialScriptStateInfo;
        };
        void* m_pStoredDataBlock { nullptr };
    };
    RE_VERIFY_SIZE(ZScriptC, 0x38);
    RE_VERIFY_OFFSET(ZScriptC, m_pScriptCreator, 0x30);
    RE_VERIFY_OFFSET(ZScriptC, m_pStoredDataBlock, 0x34);
}
