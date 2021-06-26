#include <Glacier/Geom/ZSNDOBJ.h>
#include <G1ConfigurationService.h>
#include <cassert>

namespace Glacier
{
    bool ZSNDOBJ::AttachToDefaultRoom(bool a1)
    {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZSNDOBJ_AttachToDefaultRoom != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZSNDOBJ_AttachToDefaultRoom != G1ConfigurationService::kNotConfiguredOption) {
            return ((bool(__thiscall*)(ZSNDOBJ*,bool))(G1ConfigurationService::G1API_FunctionAddress_ZSNDOBJ_AttachToDefaultRoom))(this, a1);
        }
        return false;
    }
}