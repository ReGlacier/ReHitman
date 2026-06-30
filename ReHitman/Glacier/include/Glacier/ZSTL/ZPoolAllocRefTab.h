#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/REFTAB.h>

namespace Glacier
{
    struct ZPoolAllocator
    {
        struct ZBlockHeader
        {
            unsigned int size;
            char *pointer;
        };

        unsigned int m_iAllocated;
        unsigned int m_iHighWaterMark;
        const char *m_pName;
        char *m_pBuffer;
        unsigned int m_iSize;
        ZPoolAllocator::ZBlockHeader *m_pBlockDataBegin;
        bool m_bNearMaxWarningPrinted;
        RE_ADD_PADDING(3);
    };

    class ZPoolAllocRefTab : public REFTAB
    {
    public:
        // vtbl (no new members)
        // members
        ZPoolAllocator* m_pAllocator;
    };
    RE_VERIFY_SIZE(ZPoolAllocRefTab, 0x20);
}