#pragma once

#include <cstdint>
#include <string>
#include <string_view>

#include <Glacier/GlacierFWD.h>

namespace Glacier
{
    template <typename T = std::uintptr_t>
    T* getInterface(std::uintptr_t baseAddr)
    {
        return ((*(T**)baseAddr));
    }

    namespace Animation
    {
        struct ActiveAnimation {};
        struct Header {};
        class ZMetaKey;
    }

    struct SBoneDefinition {};
}