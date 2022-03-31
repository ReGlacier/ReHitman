#include <Glacier/ZScriptC.h>
#include <G1ConfigurationService.h>
#include <cassert>

namespace Glacier
{
    int ZScriptC::FindScript(const char* scriptName) {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZScriptC_FindScript != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZScriptC_FindScript != G1ConfigurationService::kNotConfiguredOption) {
            return ((int(__thiscall*)(ZScriptC*, const char*))(G1ConfigurationService::G1API_FunctionAddress_ZScriptC_FindScript))(this, scriptName);
        }
        return 0;
    }

    int ZScriptC::CreateScript(int pScriptCreator) {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZScriptC_CreateScript != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZScriptC_CreateScript != G1ConfigurationService::kNotConfiguredOption) {
            return ((int(__thiscall*)(ZScriptC*, int))(G1ConfigurationService::G1API_FunctionAddress_ZScriptC_CreateScript))(this, pScriptCreator);
        }

        return 0;
    }
}