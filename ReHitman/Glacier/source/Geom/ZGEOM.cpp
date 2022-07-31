#include <Glacier/Geom/ZGEOM.h>
#include <G1ConfigurationService.h>

namespace Glacier
{
    void ZGEOM::GetRootTM(Glacier::ZMat3x3* mat, Glacier::ZVector3* pos) {
        ((void(__thiscall*)(ZGEOM*, Glacier::ZMat3x3*, Glacier::ZVector3*))(G1ConfigurationService::G1API_FunctionAddress_ZGEOM_GetRootTM))(this, mat, pos);
    }

    void ZGEOM::GetMatPos(Glacier::ZMat3x3* mat, Glacier::ZVector3* pos)
    {
        ((void(__thiscall*)(ZGEOM*, Glacier::ZMat3x3*, Glacier::ZVector3*))(G1ConfigurationService::G1API_FunctionAddress_ZGEOM_GetMatPos))(this, mat, pos);
    }

    void ZGEOM::GetRootPoint(Glacier::ZVector3* pos) {
        ((void(__thiscall*)(ZGEOM*, Glacier::ZVector3*))(G1ConfigurationService::G1API_FunctionAddress_ZGEOM_GetRootPoint))(this, pos);
    }

    void ZGEOM::GetCen(Glacier::ZVector3* pos) {
        ((void(__thiscall*)(ZGEOM*, Glacier::ZVector3*))(G1ConfigurationService::G1API_FunctionAddress_ZGEOM_GetCen))(this, pos);
    }

    void ZGEOM::GetRootMatPos(Glacier::ZMat3x3* mat, Glacier::ZVector3* pos) {
        ((void(__thiscall*)(ZGEOM*, Glacier::ZMat3x3*, Glacier::ZVector3*))(G1ConfigurationService::G1API_FunctionAddress_ZGEOM_GetRootMatPos))(this, mat, pos);
    }

    void ZGEOM::GetLocalPoint(Glacier::ZVector3 *pos) {
        ((void(__thiscall*)(ZGEOM*, Glacier::ZVector3*))(G1ConfigurationService::G1API_FunctionAddress_ZGEOM_GetLocalPoint))(this, pos);
    }

    void ZGEOM::Zvmmul(Glacier::ZVector3* pos) {
        ((void(__thiscall*)(ZGEOM*, Glacier::ZVector3*))(G1ConfigurationService::G1API_FunctionAddress_ZGEOM_Zvmmul))(this, pos);
    }

    ZGEOM* ZGEOM::RefToPtr(unsigned int REF)
    {
        return ((ZGEOM*(__cdecl*)(unsigned int))(G1ConfigurationService::G1API_FunctionAddress_ZGEOM_RefToPtr))(REF);
    }

	ZOldTypeInfo** ZGEOM::GetFactory()
	{
		return ((ZOldTypeInfo**(__stdcall*)())(G1ConfigurationService::G1API_FunctionAddress_ZGEOM_GetFactory))();
	}

    ZREF ZGEOM::GetRef() {
        return ((ZREF(__thiscall*)(ZGEOM*))(G1ConfigurationService::G1API_FunctionAddress_ZGEOM_GetRef))(this);
    }
}