#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Filesystem/Fwd.h>
#include <Glacier/Serializer/IInputStream.h>


namespace Glacier
{
    class ZIOInputStream : public IInputStream
    {
    public:
        // vtbl
        ~ZIOInputStream() override;
        uint32_t Read(void* address, const uint32_t size) override;
        
        // methods
        ZIOInputStream(IOFilesystem_t* FileSystem, IOFSHandle_t* Handle);
        
        // members
        IOFilesystem_t* m_FileSystem { nullptr };
        IOFSHandle_t* m_Handle { nullptr };
    };
    RE_VERIFY_SIZE(ZIOInputStream, 0xC); // Verified
}