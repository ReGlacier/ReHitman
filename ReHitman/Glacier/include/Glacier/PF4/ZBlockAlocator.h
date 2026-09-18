#pragma once

#include <Glacier/ReGlacier.h>

#include <cstdint>


namespace Glacier::PF4
{
    struct ZDataRef;

    // Block allocator that hands out fixed-size ZDataRef buffers. Every allocator
    // owns a contiguous run of m_MaxBlocks blocks of m_BlockSize entries inside a
    // shared ZDataRef arena; free blocks are tracked on m_Stack (PC pf4runtime).
    class ZBlockAlocator
    {
    public:
        // methods
        ZBlockAlocator();
        ~ZBlockAlocator();
        void Init(int16_t* pStack, ZDataRef* pDataRef);
        ZDataRef* Alloc();
        void Free(ZDataRef* pRef);
        bool BelongsTo(ZDataRef* pDataRef) const;

        // members
        ZDataRef* m_Data;
        short* m_Stack;
        int m_BlockSize;
        int m_MaxBlocks;
        int m_Count;
    };
    RE_VERIFY_SIZE(ZBlockAlocator, 0x14); // Confirmed
}
