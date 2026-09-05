#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    struct SRMaterialProperties
    {
        uint32_t lMaterialClassType{0u};
        uint32_t lMaterialClassIndex{0u};
        uint32_t lMaterialClassFlags{0u};
        uint32_t lMaterialSortingValue{0u};
        uint32_t lObjectType{0u};
        uint32_t lObjectSubType{0u};
        uint32_t lMaterialDescriptor{0u};
        uint32_t lNameOffset{0u}; // PC
    };
};