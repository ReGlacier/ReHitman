#include <Glacier/ZGROUP.h>
#include <Glacier/ZGEOM.h>

#include <G1ConfigurationService.h>

#include <cassert>

namespace Glacier
{
    ZGEOM* ZGROUP::CreateGeom(const char* name, int typeId, bool unk3)
    {
        assert(G1ConfigurationService::GetInstance().G1API_FunctionAddress_ZGROUP_CreateGeom != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::GetInstance().G1API_FunctionAddress_ZGROUP_CreateGeom != G1ConfigurationService::kNotConfiguredOption)
        {
            using ZGROUP_CreateGeomFn = ZGEOM*(__thiscall*)(ZGROUP*, const char*, int, bool);
            auto ZGROUP_CreateGeomImpl = (ZGROUP_CreateGeomFn)G1ConfigurationService::GetInstance().G1API_FunctionAddress_ZGROUP_CreateGeom;
            return ZGROUP_CreateGeomImpl(this, name, typeId, unk3);
        }

        return nullptr;
    }
}