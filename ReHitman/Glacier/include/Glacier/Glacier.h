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
        struct ActiveAnimation;
        struct Manager;
        struct Header;
        class ZMetaKey;
    }
}

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#   define GLACIER_API __declspec(dllexport)
#else
#   define GLACIER_API
#endif
