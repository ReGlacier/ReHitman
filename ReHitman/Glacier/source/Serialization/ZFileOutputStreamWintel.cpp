#include <Glacier/Serializer/ZFileOutputStreamWintel.h>
#include <Glacier/Filesystem/ZSysFile.h>


namespace Glacier
{
    // PC 0x45A0D0.
    ZFileOutputStreamWintel::ZFileOutputStreamWintel(const char* psFileName)
        : m_Handle(g_pSysFile->Create(psFileName))
    {
    }

    // PC 0x459FF0.
    ZFileOutputStreamWintel::~ZFileOutputStreamWintel()
    {
        g_pSysFile->Close(m_Handle);
    }

    // PC 0x459FB0.
    uint32_t ZFileOutputStreamWintel::Write(const void* pAddr, const uint32_t lSize)
    {
        if (m_Handle == nullptr || m_Error)
            return 0;

        // PC leaves the stale handle in the return register once m_Error is
        // latched; callers only test for success, so 0 is returned instead.
        m_Error = g_pSysFile->WriteTo(m_Handle, const_cast<void*>(pAddr), static_cast<int>(lSize)) ? 0u : 1u;
        return m_Error ? 1u : 0u;
    }
}
