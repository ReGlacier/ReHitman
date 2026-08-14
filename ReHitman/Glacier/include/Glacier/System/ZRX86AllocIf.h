#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZOffsetAlloc.h>
#include <cstdint>


namespace Glacier
{
    // Simple tracker on PC, on XBox and later - lock-free allocator wrapper
    struct ZRX86AllocIf
    {
        // methods
        ZRX86AllocIf(uint32_t lMaxNumObjects);
        ~ZRX86AllocIf();

        // members
        uint32_t m_lMaxNumObjects;
        ZOffsetAlloc m_Allocator;
    };
    RE_VERIFY_SIZE(ZRX86AllocIf, 0x18); // Verified PC
}