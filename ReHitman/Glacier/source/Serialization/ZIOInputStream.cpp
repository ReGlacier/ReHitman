#include <Glacier/Serializer/ZIOInputStream.h>
#include <Glacier/Filesystem/IOFilesystem_t.h>


namespace Glacier
{
    ZIOInputStream::ZIOInputStream(IOFilesystem_t* FileSystem, IOFSHandle_t* Handle)
        : m_FileSystem(FileSystem)
        , m_Handle(Handle)
    {
    }
    
    ZIOInputStream::~ZIOInputStream()
    {
        if (m_FileSystem)
        {
            m_FileSystem->close(m_Handle);
        }

        m_FileSystem = nullptr;
        m_Handle = nullptr;
    }

    uint32_t ZIOInputStream::Read(void* address, const uint32_t size)
    {
        return m_FileSystem->read(m_Handle, address, size);
    }
}