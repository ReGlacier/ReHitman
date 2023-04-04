#include <Glacier/Geom/ZENVIRONMENT.h>
#include <G1ConfigurationService.h>
#include <cassert>

namespace Glacier {
    void ZENVIRONMENT::ToggleColor(bool value) {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZENVIRONMENT_ToggleColor != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZENVIRONMENT_ToggleColor != G1ConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(ZENVIRONMENT*,bool))G1ConfigurationService::G1API_FunctionAddress_ZENVIRONMENT_ToggleColor)(this, value);
        }
    }

    void ZENVIRONMENT::SetDiffuseColor(unsigned int c1, unsigned int c2) {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZENVIRONMENT_SetDiffuseColor != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZENVIRONMENT_SetDiffuseColor != G1ConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(ZENVIRONMENT*,unsigned int,unsigned int))G1ConfigurationService::G1API_FunctionAddress_ZENVIRONMENT_SetDiffuseColor)(this,c1,c2);
        }
    }
}