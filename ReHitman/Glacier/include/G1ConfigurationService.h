#pragma once

#include <cstdint>

namespace Glacier
{
    class G1ConfigurationService final
    {
        G1ConfigurationService() = default;
    public:
        G1ConfigurationService(const G1ConfigurationService&) noexcept = delete;
        G1ConfigurationService(G1ConfigurationService&&)      noexcept = delete;

        // Singleton
        static G1ConfigurationService& GetInstance();

        // Consts
        static constexpr std::intptr_t kNotConfiguredOption = 0x0;

        // API Configuration
        std::intptr_t G1API_FunctionAddress_ZGROUP_CreateGeom = kNotConfiguredOption;
        std::intptr_t G1API_FunctionAddress_ZHumanBoid_SetTarget = kNotConfiguredOption;
    };
}