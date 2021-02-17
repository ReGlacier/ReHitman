#include <Glacier/Geom/ZGEOM.h>
#include <G1ConfigurationService.h>

namespace Glacier
{
    void ZGEOM::GetMatPos(Glacier::ZMat3x3* mat, Glacier::ZVector3* pos)
    {
        ((void(__thiscall*)(ZGEOM*, Glacier::ZMat3x3*, Glacier::ZVector3*))(G1ConfigurationService::G1API_FunctionAddress_ZGEOM_GetMatPos))(this, mat, pos);
    }

    ZGEOM* ZGEOM::RefToPtr(unsigned int REF)
    {
        return ((ZGEOM*(__cdecl*)(unsigned int))(G1ConfigurationService::G1API_FunctionAddress_ZGEOM_RefToPtr))(REF);
    }

    ZREF ZGEOM::GetRef() {
        return ((ZREF(__thiscall*)(ZGEOM*))(G1ConfigurationService::G1API_FunctionAddress_ZGEOM_GetRef))(this);
    }
}