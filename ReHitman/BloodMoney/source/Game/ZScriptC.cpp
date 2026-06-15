#include <BloodMoney/Game/ZScriptC.h>
#include <G1ConfigurationService.h>
#include <cassert>

namespace Hitman::BloodMoney
{
    SCRIPTCREATOR* ZScriptC::FindScript(const char* scriptName) {
        assert(Glacier::G1ConfigurationService::G1API_FunctionAddress_ZScriptC_FindScript != Glacier::G1ConfigurationService::kNotConfiguredOption);
        if (Glacier::G1ConfigurationService::G1API_FunctionAddress_ZScriptC_FindScript != Glacier::G1ConfigurationService::kNotConfiguredOption) {
            return ((SCRIPTCREATOR * (__thiscall*)(ZScriptC*, const char*))(Glacier::G1ConfigurationService::G1API_FunctionAddress_ZScriptC_FindScript))(this, scriptName);
        }
        return 0;
    }

    SCRIPTCREATOR* ZScriptC::CreateScript(SCRIPTCREATOR* pScriptCreator) {
        assert(Glacier::G1ConfigurationService::G1API_FunctionAddress_ZScriptC_CreateScript != Glacier::G1ConfigurationService::kNotConfiguredOption);
        if (Glacier::G1ConfigurationService::G1API_FunctionAddress_ZScriptC_CreateScript != Glacier::G1ConfigurationService::kNotConfiguredOption) {
            return ((SCRIPTCREATOR * (__thiscall*)(ZScriptC*, SCRIPTCREATOR*))(Glacier::G1ConfigurationService::G1API_FunctionAddress_ZScriptC_CreateScript))(this, pScriptCreator);
        }

        return 0;
    }
}