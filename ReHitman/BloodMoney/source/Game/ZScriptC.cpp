#include <BloodMoney/Game/ZScriptC.h>
#include <Glacier/EventBase/ZScheduledScript.h>
#include <G1ConfigurationService.h>
#include <cassert>

namespace Hitman::BloodMoney
{
    Glacier::SCRIPTCREATOR* ZScriptC::FindScript(const char* scriptName) 
    {
        assert(Glacier::G1ConfigurationService::G1API_FunctionAddress_ZScriptC_FindScript != Glacier::G1ConfigurationService::kNotConfiguredOption);
        if (Glacier::G1ConfigurationService::G1API_FunctionAddress_ZScriptC_FindScript != Glacier::G1ConfigurationService::kNotConfiguredOption) {
            return ((Glacier::SCRIPTCREATOR * (__thiscall*)(ZScriptC*, const char*))(Glacier::G1ConfigurationService::G1API_FunctionAddress_ZScriptC_FindScript))(this, scriptName);
        }
        return 0;
    }

    Glacier::SCRIPTCREATOR* ZScriptC::CreateScript(Glacier::SCRIPTCREATOR* pScriptCreator)
    {
        assert(Glacier::G1ConfigurationService::G1API_FunctionAddress_ZScriptC_CreateScript != Glacier::G1ConfigurationService::kNotConfiguredOption);
        if (Glacier::G1ConfigurationService::G1API_FunctionAddress_ZScriptC_CreateScript != Glacier::G1ConfigurationService::kNotConfiguredOption) {
            return ((Glacier::SCRIPTCREATOR * (__thiscall*)(ZScriptC*, Glacier::SCRIPTCREATOR*))(Glacier::G1ConfigurationService::G1API_FunctionAddress_ZScriptC_CreateScript))(this, pScriptCreator);
        }

        return 0;
    }

    void ZScriptC::FrameUpdate()
    {
        SchedUpdate();
        DeactivateFrameUpdate();
    }

    int ZScriptC::Command(Glacier::ZMSGID command, Glacier::ZDATA data)
    {
        // TODO: Finish me
        return 0;
    }

    void ZScriptC::SchedUpdate()
    {
        auto* pRunningScriptUserData = static_cast<Glacier::ZScheduledScript*>(m_pScheduleEvent)->GetUserData();
        if (!pRunningScriptUserData)
        {
            DeactivateFrameUpdate();
            m_lRoutCases &= ~0x20;
            ZEventBase::Remove();
            return;
        }
        
        // TODO: Finish me
    }
}