#include <Glacier/Geom/ZEntityLocator.h>
#include <G1ConfigurationService.h>

namespace Glacier
{
    ZGROUP* ZEntityLocator::ParentGroup() {
        return ((ZGROUP*(__thiscall*)(ZEntityLocator*))(G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_ParentGroup))(this);
    }

    void ZEntityLocator::SetName(const char* name) {
        ((void(__thiscall*)(ZEntityLocator*, const char*))(G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_SetName))(this, name);
    }

    bool ZEntityLocator::SetPrim(int primId)
    {
        return ((bool(__thiscall*)(ZEntityLocator*, int))(G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_SetPrim))(this, primId);
    }

    void ZEntityLocator::GetMatPos(Glacier::ZMat3x3* mat, Glacier::ZVector3* pos)
    {
        ((void(__thiscall*)(ZEntityLocator*, Glacier::ZMat3x3*, Glacier::ZVector3*))(G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_GetMatPos))(this, mat, pos);
    }

    ZEntityLocator* ZEntityLocator::Next()
    {
        return ((ZEntityLocator*(__thiscall*)(ZEntityLocator*))(G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_Next))(this);
    }

    void ZEntityLocator::SetNext(ZEntityLocator* next)
    {
        ((void(__thiscall*)(ZEntityLocator*, ZEntityLocator*))(G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_SetNext))(this, next);
    }

    ZEntityLocator* ZEntityLocator::GetPrev()
    {
        return ((ZEntityLocator*(__thiscall*)(ZEntityLocator*))(G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_GetPrev))(this);
    }

    void ZEntityLocator::SetPrev(ZEntityLocator* prev)
    {
        ((void(__thiscall*)(ZEntityLocator*, ZEntityLocator*))(G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_SetPrev))(this, prev);
    }
}