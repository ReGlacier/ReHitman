#include <Glacier/Filesystem/IOFSAsyncAccess_t.h>
#include <Glacier/Filesystem/ZSysFile.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZUniMemory.h>
#include <cstring>


namespace Glacier
{
    namespace
    {
        constexpr uint32_t kAsyncReadBufferSize = 0x8000;

        AsyncFd* GetAsyncFd(ImpNativeFd_t* fd)
        {
            return fd ? reinterpret_cast<AsyncFd*>(reinterpret_cast<char*>(fd) - offsetof(AsyncFd, m_Buffer)) : nullptr;
        }

        ImpNativeFd_t* ToNativeFd(AsyncFd* fd)
        {
            return fd ? reinterpret_cast<ImpNativeFd_t*>(&fd->m_Buffer[0]) : nullptr;
        }

        bool BufferContainsOffset(const AsyncFd::SBuffer& buffer, uint32_t sectorSize, uint32_t offset)
        {
            if (!buffer.m_iValidBytes)
            {
                return false;
            }

            const auto bufferOffset = static_cast<int32_t>((offset & ~(sectorSize - 1)) / sectorSize - buffer.m_iStartSector);
            return bufferOffset >= 0 && sectorSize * static_cast<uint32_t>(bufferOffset) < buffer.m_iValidBytes;
        }
    }

    bool AsyncFd::IsValid()
    {
        return false;
    }

    void AsyncFd::Wait()
    {
    }

    bool AsyncFd::GetStatus()
    {
        return true;
    }

    void AsyncFd::BeginReading(uint32_t, uint32_t)
    {
    }

    bool AsyncFd::OpenHandle(const char*)
    {
        return false;
    }

    void AsyncFd::CloseHandle()
    {
    }

    bool AsyncFd_Generic::OpenHandle(const char* szFile)
    {
        m_hHandle = g_pSysFile->Open(szFile);
        if (!m_hHandle)
        {
            return false;
        }

        m_iSectorSize = 64;
        m_pAllocation = static_cast<char*>(ZUniMemory::Allocate(0x10040));

        const auto aligned = reinterpret_cast<uintptr_t>(m_pAllocation + m_iSectorSize) & ~(static_cast<uintptr_t>(m_iSectorSize) - 1);
        m_Buffer[0].m_pData = reinterpret_cast<char*>(aligned);
        m_Buffer[1].m_pData = m_Buffer[0].m_pData + kAsyncReadBufferSize;

        return true;
    }

    bool AsyncFd_Generic::IsValid()
    {
        return m_hHandle != nullptr;
    }

    void AsyncFd_Generic::CloseHandle()
    {
        if (m_hHandle)
        {
            g_pSysFile->Close(m_hHandle);
            m_hHandle = nullptr;
        }

        if (m_pAllocation)
        {
            ZUniMemory::Free(m_pAllocation);
            m_pAllocation = nullptr;
        }
    }

    void AsyncFd_Generic::BeginReading(uint32_t iBuffer, uint32_t iStartSector)
    {
        const auto offset = iStartSector * m_iSectorSize;
        auto bytesToRead = m_iSize - offset;
        if (bytesToRead > kAsyncReadBufferSize)
        {
            bytesToRead = kAsyncReadBufferSize;
        }

        if (offset <= m_iSize && bytesToRead)
        {
            g_pSysFile->Seek(m_hHandle, offset);

            auto& buffer = m_Buffer[iBuffer];
            buffer.m_iStartSector = iStartSector;
            m_iNextSectorHint = bytesToRead / m_iSectorSize + iStartSector;
            buffer.m_iValidBytes = g_pSysFile->ReadFrom(m_hHandle, buffer.m_pData, bytesToRead);
        }
    }

    ImpNativeFd_t* IOFSAsyncAccess_t::nativeOpen(const char* file, char*)
    {
        auto* fd = ZUniMemory::New<AsyncFd_Generic>();
        if (fd->OpenHandle(file))
        {
            fd->m_iSize = g_pSysFile->GetSize(file, false);
            fd->m_iOutputOffset = 0;
            fd->m_Buffer[0].m_iValidBytes = 0;
            fd->m_Buffer[1].m_iValidBytes = 0;
            fd->m_iReading = 0;
            fd->m_iOutputBuffer = 0;
            return ToNativeFd(fd);
        }

        ZUniMemory::Delete(fd);
        return nullptr;
    }

    int IOFSAsyncAccess_t::nativeClose(ImpNativeFd_t* pFD)
    {
        auto* fd = GetAsyncFd(pFD);
        if (fd)
        {
            fd->CloseHandle();
            ZUniMemory::Delete(fd);
        }

        return 0;
    }

    int IOFSAsyncAccess_t::nativeFdValid(ImpNativeFd_t* pFD)
    {
        auto* fd = GetAsyncFd(pFD);
        return fd ? fd->IsValid() : 0;
    }

    int IOFSAsyncAccess_t::nativeTell(ImpNativeFd_t* pFD)
    {
        auto* fd = GetAsyncFd(pFD);
        return fd ? fd->m_iOutputOffset : 0;
    }

    int IOFSAsyncAccess_t::nativeSeek(ImpNativeFd_t* pFD, int offset, int whence)
    {
        auto* fd = GetAsyncFd(pFD);
        if (!fd)
        {
            return 0;
        }

        uint32_t newOffset = 0;
        if (whence == SEEK_SET)
        {
            newOffset = offset;
        }
        else if (whence == SEEK_CUR)
        {
            newOffset = fd->m_iOutputOffset + offset;
        }
        else if (whence < 3)
        {
            newOffset = fd->m_iSize - offset;
        }

        if (fd->m_iReading)
        {
            fd->Wait();
        }

        if (BufferContainsOffset(fd->m_Buffer[0], fd->m_iSectorSize, newOffset))
        {
            fd->m_iOutputBuffer = 0;
            fd->m_iOutputOffset = newOffset;
        }
        else if (BufferContainsOffset(fd->m_Buffer[1], fd->m_iSectorSize, newOffset))
        {
            fd->m_iOutputOffset = newOffset;
            fd->m_iOutputBuffer = 1;
        }
        else
        {
            fd->Wait();
            fd->m_Buffer[0].m_iValidBytes = 0;
            fd->m_Buffer[1].m_iValidBytes = 0;
            fd->m_iOutputOffset = newOffset;
        }

        return fd->m_iOutputOffset;
    }

    int IOFSAsyncAccess_t::nativeReadTo(void* pBuffer, uint32_t lBufferSize, ImpNativeFd_t* pFD)
    {
        auto* fd = GetAsyncFd(pFD);
        if (!fd)
        {
            return 0;
        }

        auto* out = static_cast<char*>(pBuffer);
        uint32_t totalRead = 0;
        auto remaining = lBufferSize;

        while (remaining)
        {
            void* chunk = nullptr;
            const auto read = nativeReadOL(&chunk, remaining, pFD);
            if (!read)
            {
                break;
            }

            memcpy(out + totalRead, chunk, read);
            totalRead += read;
            remaining -= read;
        }

        return totalRead;
    }

    int IOFSAsyncAccess_t::nativeWrite(void*, uint32_t, ImpNativeFd_t*)
    {
        return -1;
    }

    int IOFSAsyncAccess_t::nativeEof(ImpNativeFd_t* pFD)
    {
        auto* fd = GetAsyncFd(pFD);
        return fd ? fd->m_iOutputOffset >= fd->m_iSize : 1;
    }

    bool IOFSAsyncAccess_t::CanReadOverlayed() const
    {
        return true;
    }

    int IOFSAsyncAccess_t::nativeReadOL(void** ppBuffer, uint32_t lBufferSize, ImpNativeFd_t* pFD)
    {
        auto* fd = GetAsyncFd(pFD);
        if (!fd)
        {
            *ppBuffer = nullptr;
            return 0;
        }

        if (fd->m_iReading)
        {
            fd->Wait();
        }

        auto& buffer = fd->m_Buffer[fd->m_iOutputBuffer];
        if (!BufferContainsOffset(buffer, fd->m_iSectorSize, fd->m_iOutputOffset))
        {
            auto& otherBuffer = fd->m_Buffer[1 - fd->m_iOutputBuffer];
            if (BufferContainsOffset(otherBuffer, fd->m_iSectorSize, fd->m_iOutputOffset))
            {
                fd->m_iOutputBuffer = 1 - fd->m_iOutputBuffer;
            }
            else
            {
                auto& currentBuffer = fd->m_Buffer[fd->m_iOutputBuffer];
                currentBuffer.m_iValidBytes = 0;
                fd->BeginReading(fd->m_iOutputBuffer, (fd->m_iOutputOffset & ~(fd->m_iSectorSize - 1)) / fd->m_iSectorSize);
                fd->Wait();
            }
        }

        auto& activeBuffer = fd->m_Buffer[fd->m_iOutputBuffer];
        if (!activeBuffer.m_iValidBytes)
        {
            *ppBuffer = nullptr;
            return 0;
        }

        const auto bufferOffset = fd->m_iOutputOffset - activeBuffer.m_iStartSector * fd->m_iSectorSize;
        auto bytesRead = activeBuffer.m_iValidBytes - bufferOffset;
        if (bytesRead > lBufferSize)
        {
            bytesRead = lBufferSize;
        }

        *ppBuffer = activeBuffer.m_pData + bufferOffset;
        fd->m_iOutputOffset += bytesRead;

        const auto outputBuffer = fd->m_iOutputBuffer;
        if (bytesRead + bufferOffset == activeBuffer.m_iValidBytes)
        {
            activeBuffer.m_iValidBytes = 0;
            fd->m_iOutputBuffer = 1 - fd->m_iOutputBuffer;
        }

        if (!fd->m_iReading)
        {
            if (!fd->m_Buffer[0].m_iValidBytes && outputBuffer)
            {
                fd->BeginReading(0, fd->m_iNextSectorHint);
            }
            else if (!fd->m_Buffer[1].m_iValidBytes && outputBuffer != 1)
            {
                fd->BeginReading(1, fd->m_iNextSectorHint);
            }
        }

        return bytesRead;
    }
}
