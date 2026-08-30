#pragma once

#include <Glacier/Audio/ZIOStream.h>
#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZBlockAlloc.h>

#include <cstdint>

namespace Glacier
{
    class ZIOStreamer
    {
    public:
        struct SIOStreamInfo
        {
            ZIOStream* m_pStream;
            int32_t m_lLoadSize;
            uint32_t m_lLoadOffset;
            uint32_t m_lFileHandle;
            void* m_pLoadBuffer;
            int32_t m_lStreamId;
            bool m_bMetaData;
            bool m_bUpdateStream;
            char m_padding1A[2];
        };

        ZIOStreamer();
        virtual bool Create(int _numBlocks, int _blockSize, uint32_t _maxLatency);
        virtual void Destroy();
        virtual bool SetAudioStreamFile(const char* _streamFile, const char* _localizedStreamFile);
        virtual uint32_t GetFileHandle(const char* _fileName) = 0;
        virtual char* GetMetaLoadBuffer(ZIOStream* _stream);
        virtual void Reset();
        virtual bool Busy() = 0;
        virtual void IssueRead() = 0;

        void SetMetaMem(char* _metaMemory);
        ZIOStream* AddAudioStream(int _latency, uint32_t _fileOffset, uint32_t _streamSize, bool _loop,
            uint32_t _startOffset, int _metaChunkSize, int _waveChunkSize, int _numMetaChunks, bool _localized);
        void RemoveStream(ZIOStream* _stream);
        void SetupRead();
        bool Update();

        SIOStreamInfo m_CurrentStream;
        ZBlockAlloc m_Alloc;
        ZIOStream m_Streams[4];
        int32_t m_lCurrentStream;
        int32_t m_lCurrentNumStreams;
        int32_t m_lBlockSize;
        int32_t m_lMaxLatency;
        uint32_t m_lAudioStreamFileHandle;
        int32_t m_lMetaBufferSegments;
        char* m_pMetaMem;
    };

    RE_VERIFY_SIZE(ZIOStreamer::SIOStreamInfo, 0x1C);
    RE_VERIFY_OFFSET(ZIOStreamer, m_Alloc, 0x20);
    RE_VERIFY_OFFSET(ZIOStreamer, m_Streams, 0x40);
    RE_VERIFY_OFFSET(ZIOStreamer, m_lCurrentStream, 0x220);
    RE_VERIFY_OFFSET(ZIOStreamer, m_lBlockSize, 0x228);
    RE_VERIFY_OFFSET(ZIOStreamer, m_pMetaMem, 0x238);
    RE_VERIFY_SIZE(ZIOStreamer, 0x23C);
}
