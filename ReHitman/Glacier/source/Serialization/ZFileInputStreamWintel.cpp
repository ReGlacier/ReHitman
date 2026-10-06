#include <Glacier/Serializer/ZFileInputStreamWintel.h>
#include <Glacier/Filesystem/ZSysFile.h>


namespace Glacier
{
    // PC 0x45A150.
    ZFileInputStreamWintel::ZFileInputStreamWintel(const char* psFileName)
        : m_Handle(g_pSysFile->Open(psFileName))
    {
    }

    // PC 0x45A080.
    ZFileInputStreamWintel::~ZFileInputStreamWintel()
    {
        g_pSysFile->Close(m_Handle);
    }

    // PC 0x45A040.
    uint32_t ZFileInputStreamWintel::Read(void* address, const uint32_t size)
    {
        if (m_Handle == nullptr || m_Error)
            return 0;

        const int lBytesRead = g_pSysFile->ReadFrom(m_Handle, address, size);
        m_Error = static_cast<uint32_t>(lBytesRead) != size;
        return static_cast<uint32_t>(lBytesRead);
    }
}
