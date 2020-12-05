#pragma once

#include <cstdint>

namespace Glacier
{
    class G1ConfigurationService final
    {
    public:
        // Consts
        static constexpr std::intptr_t kNotConfiguredOption = 0x0;

        // API Configuration
        static std::intptr_t G1API_FunctionAddress_ZSysMem_Alloc;
        static std::intptr_t G1API_FunctionAddress_ZSysMem_Free;
        static std::intptr_t G1API_FunctionAddress_ZGROUP_CreateGeom;
        static std::intptr_t G1API_FunctionAddress_ZGROUP_IsRoot;
        static std::intptr_t G1API_FunctionAddress_ZGEOM_GetMatPos;
        static std::intptr_t G1API_FunctionAddress_ZHumanBoid_SetTarget;
        static std::intptr_t G1API_FunctionAddress_ZBaseGeom_ParentGroup;
        static std::intptr_t G1API_FunctionAddress_ZBaseGeom_SetName;
        static std::intptr_t G1API_FunctionAddress_ZBaseGeom_SetPrim;
        static std::intptr_t G1API_FunctionAddress_ZBaseGeom_GetMatPos;
    };
}