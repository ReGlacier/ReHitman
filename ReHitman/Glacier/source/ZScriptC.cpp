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
}