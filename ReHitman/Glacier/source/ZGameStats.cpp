#include <Glacier/ZGameStats.h>
#include <G1ConfigurationService.h>
#include <assert.h>

namespace Glacier {
    void ZGameStats::IncreaseCurrentShotCount() {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZGameStats_IncreaseCurrentShotCount != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZGameStats_IncreaseCurrentShotCount != G1ConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(ZGameStats*))G1ConfigurationService::G1API_FunctionAddress_ZGameStats_IncreaseCurrentShotCount)(this);
        }
    }

    void ZGameStats::DecreaseCurrentShotCount() {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZGameStats_DecreaseCurrentShotCount != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZGameStats_DecreaseCurrentShotCount != G1ConfigurationService::kNotConfiguredOption) {
            ((void(__thiscall*)(ZGameStats*))G1ConfigurationService::G1API_FunctionAddress_ZGameStats_DecreaseCurrentShotCount)(this);
        }
    }
}