#include <Glacier/ZSTL/ZHashBase.h>
#include <G1ConfigurationService.h>
#include <cassert>

namespace Glacier
{
    void ZHash<int, SMatPos>::Put(SMatPos value, bool a2)
    {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZHash_int_SMatPos_Put != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZHash_int_SMatPos_Put != G1ConfigurationService::kNotConfiguredOption)
        {
            ((void(__thiscall*)(ZHashBase*, SMatPos, bool))(G1ConfigurationService::G1API_FunctionAddress_ZHash_int_SMatPos_Put))(this, value, a2);
        }
    }

    void ZHash<int, SMatPos>::Clear() {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZHash_int_SMatPos_Clear != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZHash_int_SMatPos_Clear != G1ConfigurationService::kNotConfiguredOption)
        {
            ((void(__thiscall*)(ZHashBase*))(G1ConfigurationService::G1API_FunctionAddress_ZHash_int_SMatPos_Clear))(this);
        }
    }

    SMatPos* ZHash<int, SMatPos>::Find(const int& val) const {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZHash_int_SMatPos_Find != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZHash_int_SMatPos_Find != G1ConfigurationService::kNotConfiguredOption)
        {
            return ((SMatPos*(__thiscall*)(const ZHashBase*, const int&))(G1ConfigurationService::G1API_FunctionAddress_ZHash_int_SMatPos_Find))(this, val);
        }
        return nullptr;
    }
}