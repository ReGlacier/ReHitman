#include <Glacier/Geom/ZGROUP.h>
#include <Glacier/Geom/ZGEOM.h>
#include <Glacier/Geom/ZGeomBuffer.h>
#include <Glacier/ZSysInterface.h>
#include <Glacier/ZEngineDataBase.h>

#include <G1ConfigurationService.h>

#include <cassert>

namespace Glacier
{
    STATIC_CLASS_VAR_IMPL(ZGROUP, uint32_t, m_Id, 0, 0);
    STATIC_CLASS_VAR_IMPL(ZGROUP, uint32_t, m_Mask, 0, 0);

    ZGEOM* ZGROUP::CreateGeom(const char* name, uint32_t typeId, bool unk3)
    {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZGROUP_CreateGeom != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZGROUP_CreateGeom != G1ConfigurationService::kNotConfiguredOption)
        {
            using ZGROUP_CreateGeomFn = ZGEOM*(__thiscall*)(ZGROUP*, const char*, uint32_t, bool);
            auto ZGROUP_CreateGeomImpl = (ZGROUP_CreateGeomFn)G1ConfigurationService::G1API_FunctionAddress_ZGROUP_CreateGeom;
            return ZGROUP_CreateGeomImpl(this, name, typeId, unk3);
        }

        return nullptr;
    }

    bool ZGROUP::IsRoot()
    {
        assert(G1ConfigurationService::G1API_FunctionAddress_ZGROUP_IsRoot != G1ConfigurationService::kNotConfiguredOption);
        if (G1ConfigurationService::G1API_FunctionAddress_ZGROUP_IsRoot != G1ConfigurationService::kNotConfiguredOption)
        {
            return ((bool(__thiscall*)(ZGROUP*))(G1ConfigurationService::G1API_FunctionAddress_ZGROUP_IsRoot))(this);
        }

        return true;
    }

    ZGEOM* ZGROUP::CreateResourceGeom(const char* pName, uint32_t iGeomResourceId, uint32_t lGeomClassType, bool bCalcMinMax)
    {
        if (g_pEngineData->ResourcesDisabled())
        {
            return CreateGeom(pName, lGeomClassType, bCalcMinMax);
        }
        
        auto* pResourceBaseGeom = ZGeomBuffer::Instance().AllocResourceGeom(pName, iGeomResourceId, lGeomClassType);
        if (!pResourceBaseGeom)
        {
            return nullptr;
        }

        AttachGeom(pResourceBaseGeom, bCalcMinMax);
        return pResourceBaseGeom->GetGeom();
    }
}
