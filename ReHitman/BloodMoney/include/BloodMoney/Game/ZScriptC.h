#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ScriptEngine/Fwd.h>
#include <Glacier/CBaseEvent.h>
#include <Glacier/Geom/ZGEOM.h>
#include <cstdint>

namespace Hitman::BloodMoney
{
    class ZScriptC : public Glacier::CBaseEvent<Glacier::ZGEOM>
    {
    public:
        static constexpr const char* Name = "ScriptC";

        // vtbl
        void PostSave(Glacier::ISerializerStream& stream) override;
        bool PostLoad(Glacier::ISerializerStream& stream) override;
        const Glacier::RTP::ZPropertyInfo& GetProperties() const override;
        void Init2() override;
        void PostInit() override;
        void FrameUpdate() override;
        int Command(Glacier::ZMSGID command, Glacier::ZDATA data) override;
        void SchedUpdate() override;

        // methods
        ZScriptC();
        Glacier::SCRIPTCREATOR* FindScript(const char* scriptName);
        Glacier::SCRIPTCREATOR* CreateScript(Glacier::SCRIPTCREATOR* pScriptCreator);

    public: // data
        Glacier::SCRIPTCREATOR* m_pScriptCreator;
        Glacier::ScriptStateInfo* m_pInitialScriptStateInfo;
    };
    RE_VERIFY_SIZE(ZScriptC, 0x38);
    RE_VERIFY_OFFSET(ZScriptC, m_pScriptCreator, 0x30);
    RE_VERIFY_OFFSET(ZScriptC, m_pInitialScriptStateInfo, 0x34);
}