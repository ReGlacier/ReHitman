#include <Glacier/Audio/ZIOStream.h>
#include <Glacier/ZUniAssert.h>

#include <cstring>

namespace Glacier
{
    const char* ZIOStream::GetMetaData()
    {
        if (!m_bMetaDataReady)
            return nullptr;

        m_bMetaDataReady = false;
        return m_pMetaMem;
    }

    void ZIOStream::SetStartOffset(uint32_t _offsetInBytes, uint32_t _waveChunkSize, uint32_t _metaChunkSize, uint32_t _numMetaChunksInStream)
    {
        ZASSERT(static_cast<uint32_t>(m_lBytesLeftInStream) >= _offsetInBytes);

        if (_numMetaChunksInStream)
        {
            const uint32_t alignedOffset = _offsetInBytes & 0xFFFFC000;
            m_lFilePointer += alignedOffset;
            m_lBytesLeftInStream -= alignedOffset;
            m_lBytesToSkip = _offsetInBytes & 0x3FFF;
            m_lBytesLoaded += alignedOffset;

            const uint32_t skippedMetaChunks = _numMetaChunksInStream == 1
                ? 1
                : (_offsetInBytes + _waveChunkSize - 1) / _waveChunkSize;

            m_lFilePointer += skippedMetaChunks * _metaChunkSize;
            m_lBlocksLeft -= skippedMetaChunks;
            m_lBytesLoaded += skippedMetaChunks * _metaChunkSize;
            m_lDataChunkCounter = (_offsetInBytes >> 14) % m_lDataChunks + 1;
        }
        else
        {
            m_lFilePointer += _offsetInBytes;
            m_lBytesLeftInStream -= _offsetInBytes;
            m_lBytesLoaded += _offsetInBytes;
        }
    }

    int ZIOStream::ReadFromStream(char** _streamData, int _requestedSize, int& _bytesRead)
    {
        if (!m_lBytesLeftInStream)
        {
            _bytesRead = 0;
            *_streamData = nullptr;
            return 3;
        }

        ZASSERT(m_lBytesLeftInStream > 0);
        if (!m_lBytesReady)
        {
            _bytesRead = 0;
            *_streamData = nullptr;
            return 0;
        }

        int readSize = _requestedSize < m_lBytesReady ? _requestedSize : m_lBytesReady;
        if (!m_lCurrentBlockSize)
        {
            m_lCurrentReadPointer = 0;
            if (m_lHandleLastBlock != -1 && m_lHandleLastBlock == m_lCurrentReadHandle && m_bLoopStream)
            {
                m_lLastBlock = 0;
                m_lHandleLastBlock = -1;
            }

            if (m_lCurrentReadHandle != -1)
                m_lCurrentReadHandle = m_pAlloc->m_pFreeMemTab[m_lCurrentReadHandle];
            if (m_lCurrentReadHandle == -1)
                m_lCurrentReadHandle = m_lMemHandle;

            m_lCurrentBlockSize = m_pAlloc->m_lBlockSize;
            if (m_lHandleLastBlock != -1 && m_lHandleLastBlock == m_lCurrentReadHandle)
                m_lCurrentBlockSize = m_lLastBlock;
            if (m_bLoopStream || m_lHandleLastBlock == -1)
                ++m_lBlocksLeft;
        }

        if (m_lCurrentBlockSize < readSize)
            readSize = m_lCurrentBlockSize;

        *_streamData = m_lCurrentReadHandle == -1
            ? nullptr
            : m_pAlloc->m_pBlockMemAligned + m_lCurrentReadHandle * m_pAlloc->m_lBlockSize + m_lCurrentReadPointer;
        m_lCurrentReadPointer += readSize;

        if (!m_bLoopStream)
        {
            ZASSERT(m_lBytesLeftInStream >= readSize);
            m_lBytesLeftInStream -= readSize;
        }

        m_lBytesReady -= readSize;
        m_lCurrentBlockSize -= readSize;
        _bytesRead = readSize;
        return m_lBytesLeftInStream ? 1 : 2;
    }

    int ZIOStream::GetData(char* _destination, int _requestedSize, int& _bytesRead, char _filler)
    {
        _bytesRead = 0;
        int result = 1;
        bool endOfStream = false;

        while (_requestedSize && (result == 1 || result == 2) && !endOfStream)
        {
            char* streamData = nullptr;
            int read = 0;
            result = ReadFromStream(&streamData, _requestedSize, read);

            if (result == 0 || result == 3)
            {
                _bytesRead = 0;
                return result;
            }
            if (result == 2)
                endOfStream = true;

            if (_destination)
                std::memcpy(_destination + _bytesRead, streamData, read);
            _requestedSize -= read;
            _bytesRead += read;
        }

        if (endOfStream && _requestedSize)
            std::memset(_destination + _bytesRead, _filler, _requestedSize);
        return result;
    }
}
