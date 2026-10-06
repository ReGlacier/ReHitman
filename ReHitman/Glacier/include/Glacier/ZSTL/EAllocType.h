#pragma once


namespace Glacier
{
    enum EAllocType : int
    {
        RENDERCPU_MEM = 0x0,
        RENDERPRIMACCESS_MEM = 0x1,
        DEFAULT_MEM = 0x2,
        SLOW_MEM = 0x3,
        FAST_MEM = 0x4,
        STATIC_MEM = 0x5,
        // PHYSICS_MEM = 0x6, // in MiniNinjas, but on PC there are not

        END_OF_ALLOCATOR_TYPES,
    };
}