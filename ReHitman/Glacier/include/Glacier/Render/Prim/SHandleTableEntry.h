#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    struct SHandleTableEntry
    {
        uint32_t lOffset;
        uint32_t lSize;
        uint32_t lRefCount;
        RE_ADD_PADDING(4);
    };
    RE_VERIFY_SIZE(SHandleTableEntry, 0x10);
}