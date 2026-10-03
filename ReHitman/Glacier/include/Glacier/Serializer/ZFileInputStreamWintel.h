#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Serializer/IInputStream.h>

#include <cstdint>


namespace Glacier
{
    /**
     * IInputStream backed by a raw ZSysFile handle (PC/Wintel).
     *
     * The file is opened in the constructor through g_pSysFile; a failed or
     * short read latches m_Error so subsequent reads return 0.
     */
    struct ZFileInputStreamWintel : public IInputStream
    {
        // vtbl
        /** Closes the underlying file handle. */
        ~ZFileInputStreamWintel() override;

        /** Reads from the file, returning the number of bytes read. */
        uint32_t Read(void* address, const uint32_t size) override;

        // methods
        /** Opens psFileName for reading via g_pSysFile. */
        ZFileInputStreamWintel(const char* psFileName);

        // members
        /** File handle returned by g_pSysFile->Open (nullptr on failure). */
        void* m_Handle{nullptr};

        /** Latched once the open succeeded but a read came up short. */
        bool m_Error{false};
    };
    RE_VERIFY_SIZE(ZFileInputStreamWintel, 0xC); // Verified in PC
}
