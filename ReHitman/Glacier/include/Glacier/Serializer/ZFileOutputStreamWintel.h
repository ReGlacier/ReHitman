#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Serializer/IOutputStream.h>

#include <cstdint>


namespace Glacier
{
    /**
     * IOutputStream backed by a raw ZSysFile handle (PC/Wintel).
     *
     * The file is created in the constructor through g_pSysFile; a failed
     * write latches m_Error so subsequent writes are skipped.
     */
    struct ZFileOutputStreamWintel : public IOutputStream
    {
        // vtbl
        /** Closes the underlying file handle. */
        ~ZFileOutputStreamWintel() override;

        /** Writes to the file, returning 0 on success and 1 on failure. */
        uint32_t Write(const void* pAddr, const uint32_t lSize) override;

        // methods
        /** Creates psFileName for writing via g_pSysFile. */
        ZFileOutputStreamWintel(const char* psFileName);

        // members
        /** File handle returned by g_pSysFile->Create (nullptr on failure). */
        void* m_Handle{nullptr};

        /** Latched once a write through g_pSysFile failed. */
        bool m_Error{false};
    };
    RE_VERIFY_SIZE(ZFileOutputStreamWintel, 0xC); // Verified in PC
}
