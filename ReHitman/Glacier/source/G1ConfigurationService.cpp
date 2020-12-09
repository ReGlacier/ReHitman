#include <G1ConfigurationService.h>

namespace Glacier
{
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZSysMem_Alloc         = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZSysMem_Free          = G1ConfigurationService::kNotConfiguredOption;

    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZGROUP_CreateGeom     = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZGROUP_IsRoot         = G1ConfigurationService::kNotConfiguredOption;

    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZGEOM_GetMatPos       = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZGEOM_RefToPtr        = G1ConfigurationService::kNotConfiguredOption;

    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZHumanBoid_SetTarget  = G1ConfigurationService::kNotConfiguredOption;

    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_ParentGroup = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_SetName     = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_SetPrim     = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_GetMatPos   = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_Next        = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_SetNext     = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_GetPrev     = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZBaseGeom_SetPrev     = G1ConfigurationService::kNotConfiguredOption;

    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZAction_GetActionArray = G1ConfigurationService::kNotConfiguredOption;

    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZItemContainer_FreePos = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZItemContainer_OccupyPos = G1ConfigurationService::kNotConfiguredOption;
    std::intptr_t G1ConfigurationService::G1API_FunctionAddress_ZItemContainer_IsContainerFull = G1ConfigurationService::kNotConfiguredOption;
}