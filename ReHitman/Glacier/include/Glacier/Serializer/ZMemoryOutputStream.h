#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Serializer/IOutputStream.h>
#include <Glacier/Serializer/ZOutputStreamBase.h>
#include <Glacier/ZUniAssert.h>

#include <cstddef>
#include <cstdint>


namespace Glacier
{
    struct ZOutputStream;

    /**
     * Paged in-memory output stream used as a temporary serializer sink.
     *
     * Data is appended into linked fixed-size pages and can later be copied to a
     * buffered ZOutputStream or consumed through the IOutputStream interface.
     */
    struct ZMemoryOutputStream : public ZOutputStreamBase, public IOutputStream
    {
        // types
        /** Fixed-size storage page in the memory output stream linked list. */
        struct ZHeader
        {
            // const
            /** Number of payload bytes stored in each page. */
            static constexpr size_t m_PageSize = 16352;
            
            // methods
            /** Creates an unlinked page. */
            ZHeader() 
                : m_Next{nullptr} 
            {
            }

            /** Creates a page and links it after the previous page cursor. */
            ZHeader(ZMemoryOutputStream::ZHeader* pPrev) 
            {
                m_Next = nullptr;
                pPrev->m_Next->m_Next = this;
                pPrev->m_Next = this;
            }

            /** Returns a pointer to a byte inside the page payload. */
            const uint8_t* GetAddress(uint32_t lOffset) const
            {
                ZASSERT(lOffset < m_PageSize);
                return &m_Data[lOffset];
            }

            // members
            /** Next page in the stream or nullptr for the tail page. */
            ZMemoryOutputStream::ZHeader* m_Next{nullptr};

            /** Raw payload bytes stored by this page. */
            uint8_t m_Data[m_PageSize];
        };
        RE_VERIFY_SIZE(ZHeader, 0x3FE4u); // 16356 = 16352 + sizeof(m_Next)

        // vtbl
        ~ZMemoryOutputStream() override;
        //      ZOutputStreamBase:
        /** Appends raw bytes to the paged memory buffer. */
        uint32_t WriteRaw(char* address, const uint32_t size) override;

        /** Appends bytes while remapping source offsets with the supplied mask. */
        uint32_t WriteChangeEndianness(char * address, const uint32_t size, const uint32_t mask) override;
        //      IOutputStream:
        /** Appends bytes through the generic output stream interface. */
        uint32_t Write(const void* pAddr, const uint32_t lSize) override;

        // methods
        /** Creates an empty memory stream with one allocated page. */
        ZMemoryOutputStream();

        /** Writes all buffered memory pages into another output stream. */
        void CopyTo(ZOutputStream& stream);

        // members
        /** First page in the linked page list. */
        ZMemoryOutputStream::ZHeader* m_First;

        /** Page currently receiving appended bytes. */
        ZMemoryOutputStream::ZHeader* m_Last;

        /** Write offset inside m_Last. */
        uint32_t m_Index;
    };
}
