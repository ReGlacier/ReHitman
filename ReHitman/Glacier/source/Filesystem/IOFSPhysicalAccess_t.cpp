#include <Glacier/Filesystem/IOFSPhysicalAccess_t.h>
#include <Glacier/Filesystem/ZSysFile.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZUniAssert.h>
#include <cstdio>
#include <cstring>


namespace Glacier
{
    STATIC_GLOBAL_CLASS_INSTANCE_IMPL(IOFSPhysicalAccess_t, DefaultIOFSPhysicalAccess, 0x007F2A24, IOFSPhysicalAccess_t{});

    namespace
    {
        inline FILE* GetNativeFP(ImpNativeFd_t* fd)
        {
            return fd ? *reinterpret_cast<FILE**>(fd) : nullptr;
        }
    }

    ImpNativeFd_t* IOFSPhysicalAccess_t::nativeOpen(const char* file, char* mode)
    {
        MYSTR str;
        g_pSysFile->ConvertFilename(&str, file);

        FILE* fp = fopen(str.String, mode);
        if (!fp)
            return nullptr;

        ImpNativeFd_t* fd = (ImpNativeFd_t*)ZUniMemory::Allocate(sizeof(FILE*));
        *reinterpret_cast<FILE**>(fd) = fp;
        
        return fd;
    }

    int IOFSPhysicalAccess_t::nativeClose(ImpNativeFd_t* pFD)
    {
        FILE* fp = *reinterpret_cast<FILE**>(pFD);
        auto res = fclose(fp);
        ZUniMemory::Free(pFD);
        return res;
    }

    int IOFSPhysicalAccess_t::nativeFdValid(ImpNativeFd_t* pFD)
    {
        return pFD && *reinterpret_cast<FILE**>(pFD) != nullptr;
    }

    int IOFSPhysicalAccess_t::nativeTell(ImpNativeFd_t* pFD)
    {
        if (auto* fp = GetNativeFP(pFD))
        {
            return ftell(fp);
        }

        return -1;
    }

    int IOFSPhysicalAccess_t::nativeSeek(ImpNativeFd_t* pFD, int offset, int whence)
    {
        if (auto* fp = GetNativeFP(pFD))
        {
            return fseek(fp, offset, whence);
        }

        return -1;
    }

    int IOFSPhysicalAccess_t::nativeReadTo(void* pBuffer, uint32_t lBufferSize, ImpNativeFd_t* pFD)
    {
        if (auto* fp = GetNativeFP(pFD))
        {
            return fread(pBuffer, 1u, lBufferSize, fp);
        }

        return -1;
    }

    int IOFSPhysicalAccess_t::nativeWrite(void* pBuffer, uint32_t lBufferSize, ImpNativeFd_t* pFD)
    {
        if (auto* fp = GetNativeFP(pFD))
        {
            return fwrite(pBuffer, 1u, lBufferSize, fp);
        }

        return -1;
    }

    int IOFSPhysicalAccess_t::nativeEof(ImpNativeFd_t* pFD)
    {
        if (auto* fp = GetNativeFP(pFD))
        {
            return feof(fp);
        }

        return -1;
    }

    bool IOFSPhysicalAccess_t::CanReadOverlayed() const
    {
        return false;   
    }

    int IOFSPhysicalAccess_t::nativeReadOL(void**, uint32_t, ImpNativeFd_t* pFD)
    {
        return 0;
    }

    void IOFSMemoryAccess_t::MemoryFd_t::init(uint32_t size)
    {
        buffer = static_cast<char*>(ZUniMemory::Allocate(size));
        maxOffset = size;
        curOffset = 0;
        shouldDeleteBuffer = true;
    }

    uint32_t IOFSMemoryAccess_t::MemoryFd_t::readFd(void* destbuffer, uint32_t size)
    {
        auto readable = static_cast<uint32_t>(maxOffset - curOffset);
        if (size < readable)
        {
            readable = size;
        }

        if (readable)
        {
            memcpy(destbuffer, &buffer[curOffset], readable);
            curOffset += readable;
        }

        return readable;
    }

    int IOFSMemoryAccess_t::MemoryFd_t::seekFd(int offset, uint32_t whence)
    {
        if (whence)
        {
            if (whence == SEEK_CUR)
            {
                curOffset += offset;
            }
            else if (whence < 3)
            {
                curOffset = maxOffset + offset;
            }
        }
        else
        {
            curOffset = offset;
        }

        if (curOffset > maxOffset)
        {
            curOffset = maxOffset;
        }

        if (curOffset < 0)
        {
            curOffset = 0;
        }

        return curOffset;
    }

    IOFSMemoryAccess_t::IOFSMemoryAccess_t()
        : memFd{}
    {
    }

    ImpNativeFd_t* IOFSMemoryAccess_t::nativeOpen(const char* file, char* mode)
    {
        auto* fd = IOFSPhysicalAccess_t::nativeOpen(file, mode);
        if (!fd)
        {
            ZASSERT(false);
            return nullptr;
        }

        const auto size = IOFSPhysicalAccess_t::nativeSeek(fd, 0, SEEK_END) == 0 ? IOFSPhysicalAccess_t::nativeTell(fd) : -1;
        IOFSPhysicalAccess_t::nativeSeek(fd, 0, SEEK_SET);

        if (size < 0)
        {
            ZASSERT(false);
            IOFSPhysicalAccess_t::nativeClose(fd);
            return nullptr;
        }

        memFd.init(static_cast<uint32_t>(size));
        const auto read = IOFSPhysicalAccess_t::nativeReadTo(memFd.buffer, static_cast<uint32_t>(size), fd);
        ZASSERT(read == size);
        IOFSPhysicalAccess_t::nativeClose(fd);

        return &memFd;
    }

    int IOFSMemoryAccess_t::nativeClose(ImpNativeFd_t* fd)
    {
        if (fd != &memFd)
        {
            ZASSERT(false);
            return -1;
        }

        if (memFd.buffer && memFd.shouldDeleteBuffer)
        {
            ZUniMemory::Free(memFd.buffer);
            memFd.buffer = nullptr;
            memFd.curOffset = 0;
            memFd.maxOffset = 0;
        }

        return 0;
    }

    int IOFSMemoryAccess_t::nativeFdValid(ImpNativeFd_t* fd)
    {
        return fd == &memFd;
    }

    int IOFSMemoryAccess_t::nativeTell(ImpNativeFd_t* fd)
    {
        if (fd == &memFd)
        {
            return memFd.curOffset;
        }

        ZASSERT(false);
        return 0;
    }

    int IOFSMemoryAccess_t::nativeSeek(ImpNativeFd_t* fd, int offset, int whence)
    {
        if (fd == &memFd)
        {
            return memFd.seekFd(offset, static_cast<uint32_t>(whence));
        }

        ZASSERT(false);
        return 0;
    }

    int IOFSMemoryAccess_t::nativeReadTo(void* buffer, uint32_t size, ImpNativeFd_t* fd)
    {
        if (fd == &memFd)
        {
            return memFd.readFd(buffer, size);
        }

        ZASSERT(false);
        return 0;
    }

    int IOFSMemoryAccess_t::nativeWrite(void*, uint32_t, ImpNativeFd_t*)
    {
        return -1;
    }

    int IOFSMemoryAccess_t::nativeEof(ImpNativeFd_t*)
    {
        return (memFd.curOffset >> 31) + (memFd.curOffset >= static_cast<uint32_t>(memFd.maxOffset)) + (static_cast<uint32_t>(memFd.maxOffset) >> 31);
    }

    bool IOFSMemoryAccess_t::CanReadOverlayed() const
    {
        return false;
    }

    int IOFSMemoryAccess_t::nativeReadOL(void**, uint32_t, ImpNativeFd_t*)
    {
        return 0;
    }

    IOFSMemoryAccess_t::MemoryFd_t* IOFSMemoryAccess_t::useBuffer(void* pBuffer, uint32_t cbBufferSize)
    {
        memFd.buffer = static_cast<char*>(pBuffer);
        memFd.maxOffset = cbBufferSize;
        memFd.shouldDeleteBuffer = false;
        memFd.curOffset = 0;

        return &memFd;
    }
}
