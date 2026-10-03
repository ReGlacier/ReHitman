#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/ZSTL/ZBlockAlloc.h>

#include <cstdint>

namespace Glacier
{
    class ZBufferGroup;

    class ZIOStream
    {
    public:
        const char* GetMetaData();
        void SetStartOffset(uint32_t _offsetInBytes, uint32_t _waveChunkSize, uint32_t _metaChunkSize, uint32_t _numMetaChunksInStream);
        int ReadFromStream(char** _streamData, int _requestedSize, int& _bytesRead);
        int GetData(char* _destination, int _requestedSize, int& _bytesRead, char _filler);

        ZBufferGroup* m_pUserData;
        int32_t m_lReferenceCount;
        int32_t m_lStreamId;
        ZBlockAlloc* m_pAlloc;
        int32_t m_lCurrentReadPointer;
        int32_t m_lCurrentReadHandle;
        int32_t m_lBytesReady;
        int32_t m_lBytesToSkip;
        int32_t m_lCurrentBlockSize;
        int32_t m_lBlocksLeft;
        int32_t m_lNumberOfBlocks;
        int32_t m_lMemHandle;
        int32_t m_lMemHandleCurrent;
        uint32_t m_lFileHandle;
        uint32_t m_lFileOffset;
        uint32_t m_lFileLoopOffset;
        uint32_t m_lFilePointer;
        int32_t m_lBytesLeftInStream;
        int32_t m_lSizeOfStream;
        int32_t m_lBytesLoaded;
        int32_t m_lLastBlock;
        int32_t m_lHandleLastBlock;
        bool m_bLoopStream;
        bool m_bPreloadReady;
        bool m_bActive;
        bool m_bMetaDataReady;
        int32_t m_lMetaChunkSize;
        int32_t m_lDataChunks;
        int32_t m_lDataChunkCounter;
        int32_t m_lNumMetaChunks;
        int32_t m_lMetaChunkCounter;
        int32_t m_lCurrentMetaSegment;
        char* m_pMetaMem;
    };

    RE_VERIFY_OFFSET(ZIOStream, m_pAlloc, 0x0C);
    RE_VERIFY_OFFSET(ZIOStream, m_lBytesReady, 0x18);
    RE_VERIFY_OFFSET(ZIOStream, m_lFileLoopOffset, 0x3C);
    RE_VERIFY_OFFSET(ZIOStream, m_lBytesLeftInStream, 0x44);
    RE_VERIFY_OFFSET(ZIOStream, m_bLoopStream, 0x58);
    RE_VERIFY_OFFSET(ZIOStream, m_pMetaMem, 0x74);
    RE_VERIFY_SIZE(ZIOStream, 0x78);
}
