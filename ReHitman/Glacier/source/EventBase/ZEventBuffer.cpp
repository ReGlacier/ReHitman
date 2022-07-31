#include <Glacier/EventBase/ZEventBuffer.h>
#include <G1ConfigurationService.h>
#include <cassert>

namespace Glacier {
    std::intptr_t ZEventBuffer::GetGQC(Glacier::ZREF ref) {
        if (G1ConfigurationService::G1API_FunctionAddress_GetGQC != G1ConfigurationService::kNotConfiguredOption) {
            return ((std::intptr_t(__cdecl*)(Glacier::ZREF))(G1ConfigurationService::G1API_FunctionAddress_GetGQC))(ref);
        }

        assert(false && "Option G1API_FunctionAddress_GetGQC not configured!");
        return 0x0;
    }
}