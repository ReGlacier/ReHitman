#pragma once

#include <Glacier/Filesystem/IOFSPhysicalAccess_t.h>


namespace Glacier
{
    struct AsyncFd : ImpNativeFd_t
    {
        struct SBuffer
        {
            char* m_pData{};
            uint32_t m_iValidBytes{};
            uint32_t m_iStartSector{};
        };

        virtual bool IsValid();
        virtual void Wait();
        virtual bool GetStatus();
        virtual void BeginReading(uint32_t iBuffer, uint32_t iStartSector);
        virtual bool OpenHandle(const char* szFile);
        virtual void CloseHandle();

        SBuffer m_Buffer[2];
        char* m_pAllocation{};
        uint32_t m_iSectorSize{};
        uint32_t m_iOutputOffset{};
        uint32_t m_iSize{};
        uint32_t m_iReading{};
        uint32_t m_iOutputBuffer{};
        uint32_t m_iNextSectorHint{};
    };

    struct AsyncFd_Generic : AsyncFd
    {
        bool IsValid() override;
        void BeginReading(uint32_t iBuffer, uint32_t iStartSector) override;
        bool OpenHandle(const char* szFile) override;
        void CloseHandle() override;

        void* m_hHandle{};
    };

    struct IOFSAsyncAccess_t : IOFSPhysicalAccess_t
    {
        ImpNativeFd_t* nativeOpen(const char* file, char* mode) override;
        int nativeClose(ImpNativeFd_t* pFD) override;
        int nativeFdValid(ImpNativeFd_t* pFD) override;
        int nativeTell(ImpNativeFd_t* pFD) override;
        int nativeSeek(ImpNativeFd_t* pFD, int offset, int whence) override;
        int nativeReadTo(void* pBuffer, uint32_t lBufferSize, ImpNativeFd_t* pFD) override;
        int nativeWrite(void* pBuffer, uint32_t lBufferSize, ImpNativeFd_t* pFD) override;
        int nativeEof(ImpNativeFd_t* pFD) override;
        bool CanReadOverlayed() const override;
        int nativeReadOL(void** ppBuffer, uint32_t lBufferSize, ImpNativeFd_t* pFD) override;
    };

    RE_VERIFY_SIZE(AsyncFd::SBuffer, 0xC);
    RE_VERIFY_OFFSET(AsyncFd::SBuffer, m_pData, 0x0);
    RE_VERIFY_OFFSET(AsyncFd::SBuffer, m_iValidBytes, 0x4);
    RE_VERIFY_OFFSET(AsyncFd::SBuffer, m_iStartSector, 0x8);
    RE_VERIFY_SIZE(AsyncFd, 0x38);
    RE_VERIFY_OFFSET(AsyncFd, m_Buffer, 0x4);
    RE_VERIFY_OFFSET(AsyncFd, m_pAllocation, 0x1C);
    RE_VERIFY_OFFSET(AsyncFd, m_iSectorSize, 0x20);
    RE_VERIFY_OFFSET(AsyncFd, m_iOutputOffset, 0x24);
    RE_VERIFY_OFFSET(AsyncFd, m_iSize, 0x28);
    RE_VERIFY_OFFSET(AsyncFd, m_iReading, 0x2C);
    RE_VERIFY_OFFSET(AsyncFd, m_iOutputBuffer, 0x30);
    RE_VERIFY_OFFSET(AsyncFd, m_iNextSectorHint, 0x34);
    RE_VERIFY_SIZE(AsyncFd_Generic, 0x3C);
    RE_VERIFY_OFFSET(AsyncFd_Generic, m_hHandle, 0x38);
    RE_VERIFY_SIZE(IOFSAsyncAccess_t, 0x4);
}
