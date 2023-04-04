#include <Glacier/CCom.h>
#include <G1ConfigurationService.h>
#include <cassert>

namespace Glacier {
    const char* CCom::Ent::GetKey() {
        return reinterpret_cast<const char*>(this) + 0xC;
    }

    std::intptr_t CCom::Ent::GetValue() {
        return *reinterpret_cast<std::intptr_t*>(reinterpret_cast<std::intptr_t>(reinterpret_cast<const char*>(this) + 0xC + m_iKeyLength));
    }

    CCom::Ent* CCom::GetPVal(const char *psName, int iSize) {
        assert(psName != nullptr);
        assert(iSize > 0);
        assert(G1ConfigurationService::G1API_FunctionAddress_CCom_GetpVal != G1ConfigurationService::kNotConfiguredOption);

        if (!psName || !iSize) {
            return nullptr;
        }

        if (G1ConfigurationService::G1API_FunctionAddress_CCom_GetpVal != G1ConfigurationService::kNotConfiguredOption) {
            return ((CCom::Ent*(__thiscall*)(CCom*, const char*, int))G1ConfigurationService::G1API_FunctionAddress_CCom_GetpVal)(this, psName, iSize);
        }

        return nullptr;
    }
}