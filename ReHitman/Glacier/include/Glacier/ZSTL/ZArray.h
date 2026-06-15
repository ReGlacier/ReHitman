#pragma once


namespace Glacier
{
    template <typename T>
    struct ZArray {
        virtual ~ZArray();

        // Inherits ZBlockArray internal allocation structures:
        void* m_pBlockData;                              // +0x0004 (bit offset: 32)
        unsigned int m_nElementCount;                    // +0x0008 (bit offset: 64)
        unsigned int m_nBlockCapacity;                   // +0x000C (bit offset: 96)
        char _pad[12];                                   // +0x0010 (Fills out the 28-byte footprint)
    };
}