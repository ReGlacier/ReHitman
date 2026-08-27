#include <Glacier/Geom/ZSNDOBJ.h>
#include <G1ConfigurationService.h>
#include <cassert>

namespace Glacier
{
    STATIC_CLASS_VAR_IMPL(ZSNDOBJ, uint32_t, m_Id, 0, 0);
    STATIC_CLASS_VAR_IMPL(ZSNDOBJ, uint32_t, m_Mask, 0, 0);

    bool ZSNDOBJ::AttachToDefaultRoom(bool bOverride)
    {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZSNDOBJ_AttachToDefaultRoom != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZSNDOBJ_AttachToDefaultRoom != G1ConfigurationService::kNotConfiguredOption) {
            return ((bool(__thiscall*)(ZSNDOBJ*,bool))(G1ConfigurationService::G1API_FunctionAddress_ZSNDOBJ_AttachToDefaultRoom))(this, bOverride);
        }
        return false;
    }
}
