#include <Glacier/Serializer/ZCompressedInputStream.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
namespace
{
    // PC binds zalloc/zfree to Glacier::AllocMemory/Glacier::FreeMemory at the
    // ctor (0x455E10); those helpers are not reversed, so route the zlib
    // allocator through the project's unified allocator instead (same role,
    // engine-side heap in the game build).
    voidpf GlacierZipAlloc(voidpf, uInt items, uInt size)
    {
        return ZUniMemory::Allocate(static_cast<int>(items) * static_cast<int>(size));
    }

    void GlacierZipFree(voidpf, voidpf address)
    {
        ZUniMemory::Free(address);
    }
}

    // PC 0x455E10.
    ZCompressedInputStream::ZCompressedInputStream(IInputStream* stream)
        : m_Stream{stream}
    {
        m_Parameters.opaque = nullptr;
        m_Parameters.zalloc = GlacierZipAlloc;
        m_Parameters.zfree = GlacierZipFree;

        // FillBuffer(): prime the staging buffer with the first block.
        m_Parameters.next_in = m_Buffer;
        m_Parameters.avail_in = m_Stream->Read(m_Buffer, BUFFER_SIZE);

        inflateInit(&m_Parameters); // zlib 1.1.3 macro; PC passes ("1.1.3", 56).
    }

    // PC 0x455D10.
    ZCompressedInputStream::~ZCompressedInputStream()
    {
        inflateEnd(&m_Parameters);
    }

    // PC 0x455EC0.
    uint32_t ZCompressedInputStream::Read(void* address, const uint32_t size)
    {
        m_Parameters.next_out = static_cast<Bytef*>(address);
        m_Parameters.avail_out = size;

        if (size)
        {
            do
            {
                if (m_Parameters.avail_in == 0)
                {
                    m_Parameters.next_in = m_Buffer;
                    m_Parameters.avail_in = m_Stream->Read(m_Buffer, BUFFER_SIZE);
                    if (m_Parameters.avail_in == 0)
                        break; // Compressed source exhausted.
                }
            }
            while (inflate(&m_Parameters, Z_NO_FLUSH) != Z_STREAM_END && m_Parameters.avail_out);
        }

        return size - m_Parameters.avail_out;
    }
}
