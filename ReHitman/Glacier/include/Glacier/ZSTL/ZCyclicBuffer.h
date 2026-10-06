#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>

namespace Glacier
{
    /**
     * @brief Cyclic byte buffer for variable-size records (engine/zstdlib/console.cpp).
     *
     * Records are stored back-to-back inside a fixed-size heap buffer. Each record is
     * prefixed with a 4-byte aligned length; a sentinel value of -1 terminates the
     * chain and may sit before the physical end of the buffer (after a wrap).
     * When space runs out the oldest records are evicted silently.
     *
     * Used by the debug console for the output log and the command history.
     */
    class ZCyclicBuffer
    {
    public:
        // methods
        ZCyclicBuffer(int iBufferSize);
        virtual ~ZCyclicBuffer();

        /** @brief Allocates space for a record of @p iSize bytes. Returns nullptr when a record of that size can never fit. */
        void* Alloc(int iSize);
        /** @brief Returns the record with logical index @p iIndex (negative counts back from the newest), nullptr when out of range. */
        void* Get(int iIndex);
        /** @brief Logical index of the newest record. */
        int Last();
        /** @brief Logical index of the oldest record. */
        int First();
        /** @brief Number of records currently stored. */
        int Count();

    private:
        /** @brief Evicts the oldest record. */
        void FreeSpace();
        /** @brief Number of payload bytes that can still be allocated without eviction. */
        int SpaceFree();

    public:
        // members
        char* m_pBuffer;
        int m_iBufferSize;
        int m_iFirst;
        int m_iCount;
        int m_iStart;
        int m_iEnd;
    };
    RE_VERIFY_SIZE(ZCyclicBuffer, 0x1C);
    RE_VERIFY_OFFSET(ZCyclicBuffer, m_pBuffer, 0x04);
    RE_VERIFY_OFFSET(ZCyclicBuffer, m_iBufferSize, 0x08);
    RE_VERIFY_OFFSET(ZCyclicBuffer, m_iFirst, 0x0C);
    RE_VERIFY_OFFSET(ZCyclicBuffer, m_iCount, 0x10);
    RE_VERIFY_OFFSET(ZCyclicBuffer, m_iStart, 0x14);
    RE_VERIFY_OFFSET(ZCyclicBuffer, m_iEnd, 0x18);
}
