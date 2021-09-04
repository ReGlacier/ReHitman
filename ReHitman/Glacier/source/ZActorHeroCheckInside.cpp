#include <Glacier/ZActorHeroCheckInside.h>
#include <Glacier/Geom/ZGEOM.h>
#include <G1ConfigurationService.h>
#include <cassert>

namespace Glacier {
    bool ZActorHeroCheckInside::IsInside(Glacier::ZGEOM* pGeom) {
        if (!pGeom) { return false; }

        assert(G1ConfigurationService::G1API_FunctionAddress_ZActorHeroCheckInside_IsInside != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZActorHeroCheckInside_IsInside != G1ConfigurationService::kNotConfiguredOption) {
            return ((bool(__thiscall*)(ZActorHeroCheckInside*,ZGEOM*))G1ConfigurationService::G1API_FunctionAddress_ZActorHeroCheckInside_IsInside)(this, pGeom);
        }

        return false;
    }
}