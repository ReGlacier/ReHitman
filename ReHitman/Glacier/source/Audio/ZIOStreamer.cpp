#include <Glacier/Audio/ZIOStreamer.h>
#include <Glacier/ZUniMemory.h>
#include <Glacier/ZUniAssert.h>

#include <cstring>

namespace Glacier
{
    ZIOStreamer::ZIOStreamer()
        : m_Alloc(64)
        , m_lCurrentStream(0)
        , m_lCurrentNumStreams(0)
        , m_lBlockSize(0)
        , m_lMaxLatency(0)
        , m_lAudioStreamFileHandle(0)
        , m_lMetaBufferSegments(2)
        , m_pMetaMem(nullptr)
    {
        std::memset(&m_CurrentStream, 0, sizeof(m_CurrentStream));
        for (int i = 0; i < 4; ++i)
        {
            m_Streams[i].m_lStreamId = i;
            m_Streams[i].m_lMemHandle = -1;
            m_Streams[i].m_lCurrentReadHandle = -1;
            m_Streams[i].m_bActive = false;
            m_Streams[i].m_bMetaDataReady = false;
        }
    }

    bool ZIOStreamer::Create(int _numBlocks, int _blockSize, uint32_t _maxLatency)
    {
        m_CurrentStream = {};
        m_Streams[0].m_lStreamId = 0;
        m_Streams[1].m_lStreamId = 1;
        m_Streams[2].m_lStreamId = 2;
        m_Streams[3].m_lStreamId = 3;
        m_lBlockSize = _blockSize;
        m_lMaxLatency = static_cast<int32_t>(_maxLatency);
        m_Alloc.Create(_numBlocks, _blockSize);
        return true;
    }

    void ZIOStreamer::Destroy()
    {
        if (m_Alloc.m_pBlockMem)
        {
            ZUniMemory::Free(m_Alloc.m_pBlockMem);
            m_Alloc.m_pBlockMem = nullptr;
            m_Alloc.m_pBlockMemAligned = nullptr;
        }
        if (m_Alloc.m_pFreeMemTab)
        {
            ZUniMemory::Free(m_Alloc.m_pFreeMemTab);
            m_Alloc.m_pFreeMemTab = nullptr;
        }
    }

    bool ZIOStreamer::SetAudioStreamFile(const char* _streamFile, const char*)
    {
        if (!_streamFile)
            return false;
        m_lAudioStreamFileHandle = GetFileHandle(_streamFile);
        return m_lAudioStreamFileHandle != static_cast<uint32_t>(-1);
    }

    char* ZIOStreamer::GetMetaLoadBuffer(ZIOStream* _stream)
    {
        return m_pMetaMem + (_stream->m_lCurrentMetaSegment << 12);
    }

    void ZIOStreamer::Reset()
    {
        for (ZIOStream& stream : m_Streams)
            stream.m_bActive = false;
        m_CurrentStream.m_bUpdateStream = false;
    }

    void ZIOStreamer::SetMetaMem(char* _metaMemory)
    {
        m_pMetaMem = _metaMemory;
    }

    ZIOStream* ZIOStreamer::AddAudioStream(int _latency, uint32_t _fileOffset, uint32_t _streamSize, bool _loop,
        uint32_t _waveChunkSize, int _metaChunkSize, int _numMetaChunks, int _startOffset, bool)
    {
        const uint32_t requestedBytes = m_lBlockSize + 4 * _latency * m_lMaxLatency / 1000;
        uint32_t blockCount = (requestedBytes + m_lBlockSize - 1) / m_lBlockSize;
        if (_loop && blockCount == 1)
            blockCount = 2;

        ZIOStream* stream = nullptr;
        for (ZIOStream& candidate : m_Streams)
        {
            if (!candidate.m_bActive)
            {
                stream = &candidate;
                break;
            }
        }
        if (!stream)
            return nullptr;

        stream->m_lReferenceCount = 0;
        stream->m_pAlloc = &m_Alloc;
        stream->m_bLoopStream = _loop;
        stream->m_lFilePointer = _fileOffset;
        stream->m_lFileLoopOffset = _fileOffset;
        stream->m_lBytesLeftInStream = static_cast<int32_t>(_streamSize - _startOffset * _metaChunkSize);
        stream->m_lSizeOfStream = static_cast<int32_t>(_streamSize);
        stream->m_lMetaChunkSize = _metaChunkSize;
        stream->m_lDataChunks = _numMetaChunks;
        stream->m_lNumberOfBlocks = static_cast<int32_t>(blockCount);
        stream->m_bActive = true;
        ++m_lCurrentNumStreams;
        return stream;
    }

    void ZIOStreamer::RemoveStream(ZIOStream* _stream)
    {
        if (!_stream)
            return;
        _stream->m_bActive = false;
        _stream->m_lMemHandle = -1;
        --m_lCurrentNumStreams;
    }

    void ZIOStreamer::SetupRead()
    {
        ZIOStream& stream = m_Streams[m_lCurrentStream];
        if (!stream.m_lBlocksLeft)
            return;

        int loadSize = m_lBlockSize;
        const int bytesRemaining = stream.m_lSizeOfStream - stream.m_lBytesLoaded;
        m_CurrentStream.m_bUpdateStream = true;

        bool loadMetaData = false;
        if (!stream.m_lDataChunkCounter && stream.m_lMetaChunkCounter)
        {
            --stream.m_lMetaChunkCounter;
            loadMetaData = true;
        }

        ++stream.m_lDataChunkCounter;
        if (stream.m_lDataChunkCounter == stream.m_lDataChunks)
            stream.m_lDataChunkCounter = 0;

        if (bytesRemaining <= loadSize)
        {
            loadSize = bytesRemaining;
            stream.m_lLastBlock = bytesRemaining;
            stream.m_lHandleLastBlock = stream.m_lMemHandleCurrent;
        }

        if (loadMetaData)
        {
            loadSize = stream.m_lMetaChunkSize;
            m_CurrentStream.m_pLoadBuffer = GetMetaLoadBuffer(&stream);
            m_CurrentStream.m_bMetaData = true;
        }
        else
        {
            m_CurrentStream.m_pLoadBuffer = stream.m_lMemHandleCurrent == -1
                ? nullptr
                : m_Alloc.m_pBlockMemAligned + stream.m_lMemHandleCurrent * m_Alloc.m_lBlockSize;
            m_CurrentStream.m_bMetaData = false;
        }

        ZASSERT(loadSize >= 0);
        m_CurrentStream.m_pStream = &stream;
        m_CurrentStream.m_lLoadSize = loadSize;
        m_CurrentStream.m_lLoadOffset = stream.m_lFilePointer;
        m_CurrentStream.m_lFileHandle = stream.m_lFileHandle;
        m_CurrentStream.m_lStreamId = stream.m_lStreamId;

        if (loadSize > 0)
            IssueRead();
    }

    bool ZIOStreamer::Update()
    {
        if (Busy())
            return true;

        if (m_CurrentStream.m_bUpdateStream)
        {
            ZIOStream& stream = *m_CurrentStream.m_pStream;
            m_CurrentStream.m_bUpdateStream = false;

            if (m_CurrentStream.m_bMetaData)
            {
                m_CurrentStream.m_bMetaData = false;
                stream.m_bMetaDataReady = true;
            }
            else
            {
                int nextHandle = stream.m_lMemHandleCurrent;
                if (nextHandle != -1)
                    nextHandle = m_Alloc.m_pFreeMemTab[nextHandle];
                stream.m_lMemHandleCurrent = nextHandle == -1 ? stream.m_lMemHandle : nextHandle;
                stream.m_lBytesReady += m_CurrentStream.m_lLoadSize;
                --stream.m_lBlocksLeft;

                if (stream.m_lBytesToSkip)
                {
                    char* data = nullptr;
                    int skipped = 0;
                    const int bytesToSkip = stream.m_lBytesToSkip;
                    stream.ReadFromStream(&data, bytesToSkip, skipped);
                    if (skipped != bytesToSkip)
                    {
                        int skippedRemainder = 0;
                        stream.ReadFromStream(&data, bytesToSkip - skipped, skippedRemainder);
                    }
                    stream.m_lBytesToSkip = 0;
                }
            }

            stream.m_lFilePointer += m_CurrentStream.m_lLoadSize;
            stream.m_lBytesLoaded += m_CurrentStream.m_lLoadSize;
            if (stream.m_lBytesLoaded == stream.m_lSizeOfStream && stream.m_bLoopStream)
            {
                stream.m_lBytesLoaded = stream.m_lFileOffset;
                stream.m_lFilePointer = stream.m_lFileLoopOffset + stream.m_lFileOffset;
                stream.m_lDataChunkCounter = 0;
                stream.m_lMetaChunkCounter = stream.m_lNumMetaChunks;
            }
        }

        for (int checked = 0; checked < 4; ++checked)
        {
            ZIOStream& stream = m_Streams[m_lCurrentStream];
            if (stream.m_bActive && stream.m_lBlocksLeft)
            {
                SetupRead();
                return true;
            }

            if (m_CurrentStream.m_pStream)
            {
                stream.m_bPreloadReady = true;
                m_CurrentStream.m_pStream = nullptr;
            }

            m_lCurrentStream = (m_lCurrentStream + 1) % 4;
        }
        return true;
    }
}
