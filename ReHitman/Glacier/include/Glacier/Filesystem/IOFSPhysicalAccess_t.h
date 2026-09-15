#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZUniMemory.h>
#include <cstdint>

namespace Glacier
{
    struct ImpNativeFd_t {}; // it's empty

    struct StdNativeFd_t : ImpNativeFd_t
    {
        int fd { 0 };
    };

    struct IOFSPhysicalAccess_t
    {
        virtual ImpNativeFd_t* nativeOpen(const char* file, char* mode);
        virtual int nativeClose(ImpNativeFd_t* pFD);
        virtual int nativeFdValid(ImpNativeFd_t* pFD);
        virtual int nativeTell(ImpNativeFd_t* pFD);
        virtual int nativeSeek(ImpNativeFd_t* pFD, int offset, int whence);
        virtual int nativeReadTo(void* pBuffer, uint32_t lBufferSize, ImpNativeFd_t* pFD);
        virtual int nativeWrite(void* pBuffer, uint32_t lBufferSize, ImpNativeFd_t* pFD);
        virtual int nativeEof(ImpNativeFd_t* pFD);
        virtual bool CanReadOverlayed() const;
        virtual int nativeReadOL(void**, uint32_t, ImpNativeFd_t* pFD);
    };

    struct IOFSMemoryAccess_t : IOFSPhysicalAccess_t
    {
        struct MemoryFd_t : ImpNativeFd_t
        {
            void init(uint32_t size);
            uint32_t readFd(void* destbuffer, uint32_t size);
            int seekFd(int offset, uint32_t whence);

            char* buffer;
            int curOffset;
            int maxOffset;
            bool shouldDeleteBuffer;
            RE_ADD_PADDING(3);
        };

        IOFSMemoryAccess_t();

        ImpNativeFd_t* nativeOpen(const char* file, char* mode) override;
        int nativeClose(ImpNativeFd_t* pFD) override;
        int nativeFdValid(ImpNativeFd_t* pFD) override;
        int nativeTell(ImpNativeFd_t* pFD) override;
        int nativeSeek(ImpNativeFd_t* pFD, int offset, int whence) override;
        int nativeReadTo(void* pBuffer, uint32_t lBufferSize, ImpNativeFd_t* pFD) override;
        int nativeWrite(void* pBuffer, uint32_t lBufferSize, ImpNativeFd_t* pFD) override;
        int nativeEof(ImpNativeFd_t* pFD) override;
        bool CanReadOverlayed() const override;
        int nativeReadOL(void**, uint32_t, ImpNativeFd_t* pFD) override;

        virtual MemoryFd_t* useBuffer(void* pBuffer, uint32_t cbBufferSize);

        MemoryFd_t memFd;
    };

    RE_VERIFY_SIZE(IOFSMemoryAccess_t::MemoryFd_t, 0x10);
    RE_VERIFY_OFFSET(IOFSMemoryAccess_t::MemoryFd_t, buffer, 0x0);
    RE_VERIFY_OFFSET(IOFSMemoryAccess_t::MemoryFd_t, curOffset, 0x4);
    RE_VERIFY_OFFSET(IOFSMemoryAccess_t::MemoryFd_t, maxOffset, 0x8);
    RE_VERIFY_OFFSET(IOFSMemoryAccess_t::MemoryFd_t, shouldDeleteBuffer, 0xC);
    RE_VERIFY_SIZE(IOFSMemoryAccess_t, 0x14);
    RE_VERIFY_OFFSET(IOFSMemoryAccess_t, memFd, 0x4);

    STATIC_GLOBAL_CLASS_INSTANCE(IOFSPhysicalAccess_t, DefaultIOFSPhysicalAccess);
}
