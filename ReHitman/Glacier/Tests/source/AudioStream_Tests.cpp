#include <Glacier/Audio/ZIOStream.h>
#include <Glacier/Audio/ZIOStreamer.h>
#include <Glacier/Audio/ZSynth.h>
#include <Glacier/ZSTL/ZBlockAlloc.h>
#include <gtest/gtest.h>

#include <cstring>

using namespace Glacier;

namespace
{
    class TestIOStreamer final : public ZIOStreamer
    {
    public:
        uint32_t GetFileHandle(const char*) override
        {
            return 1;
        }

        bool Busy() override
        {
            return false;
        }

        void IssueRead() override
        {
        }
    };

    void PrepareStream(ZIOStream& _stream, ZBlockAlloc& _allocator, int _size)
    {
        auto* memory = static_cast<char*>(_allocator.AllocBlocks((_size + 3) / 4));
        ASSERT_NE(memory, nullptr);

        _stream = {};
        _stream.m_pAlloc = &_allocator;
        _stream.m_lMemHandle = 0;
        _stream.m_lCurrentReadHandle = -1;
        _stream.m_lCurrentBlockSize = 0;
        _stream.m_lHandleLastBlock = -1;
        _stream.m_lBytesReady = _size;
        _stream.m_lBytesLeftInStream = _size;
    }
}

TEST(AudioStream, GetDataReadsAcrossBlockChain)
{
    ZBlockAlloc allocator;
    allocator.Create(3, 4);

    ZIOStream stream;
    PrepareStream(stream, allocator, 10);
    std::memcpy(allocator.m_pBlockMemAligned, "ABCDEFGHIJ", 10);

    char output[10]{};
    int bytesRead = 0;
    const int status = stream.GetData(output, sizeof(output), bytesRead, 0);

    EXPECT_EQ(status, 2);
    EXPECT_EQ(bytesRead, 10);
    EXPECT_EQ(std::memcmp(output, "ABCDEFGHIJ", 10), 0);
    EXPECT_EQ(stream.m_lBytesReady, 0);
    EXPECT_EQ(stream.m_lBytesLeftInStream, 0);
}

TEST(AudioStream, GetDataPadsRequestAtEndOfStream)
{
    ZBlockAlloc allocator;
    allocator.Create(1, 4);

    ZIOStream stream;
    PrepareStream(stream, allocator, 3);
    std::memcpy(allocator.m_pBlockMemAligned, "STR", 3);

    char output[6]{};
    int bytesRead = 0;
    const int status = stream.GetData(output, sizeof(output), bytesRead, 0x7F);

    EXPECT_EQ(status, 2);
    EXPECT_EQ(bytesRead, 3);
    EXPECT_EQ(std::memcmp(output, "STR", 3), 0);
    EXPECT_EQ(static_cast<unsigned char>(output[3]), 0x7F);
    EXPECT_EQ(static_cast<unsigned char>(output[4]), 0x7F);
    EXPECT_EQ(static_cast<unsigned char>(output[5]), 0x7F);
}

TEST(AudioStream, ReadFromStreamReportsNotReadyAndAlreadyAtEnd)
{
    ZIOStream stream{};
    stream.m_lBytesLeftInStream = 8;

    char* data = reinterpret_cast<char*>(1);
    int bytesRead = -1;
    EXPECT_EQ(stream.ReadFromStream(&data, 4, bytesRead), 0);
    EXPECT_EQ(data, nullptr);
    EXPECT_EQ(bytesRead, 0);

    stream.m_lBytesLeftInStream = 0;
    data = reinterpret_cast<char*>(1);
    bytesRead = -1;
    EXPECT_EQ(stream.ReadFromStream(&data, 4, bytesRead), 3);
    EXPECT_EQ(data, nullptr);
    EXPECT_EQ(bytesRead, 0);
}

TEST(AudioStreamer, AddAudioStreamInitializesExtractionGeometry)
{
    TestIOStreamer streamer;
    ASSERT_TRUE(streamer.Create(8, 8, 1000));

    char metadata[0x8000]{};
    streamer.SetMetaMem(metadata);
    ZBufferGroup group{};
    streamer.m_Streams[0].m_pUserData = &group;

    ZIOStream* stream = streamer.AddAudioStream(8, 0x120, 24, false, 0, 4, 16, 2, false);

    ASSERT_NE(stream, nullptr);
    EXPECT_EQ(stream->m_pUserData, &group);
    EXPECT_EQ(stream->m_lFilePointer, 0x120u);
    EXPECT_EQ(stream->m_lFileOffset, 0x120u);
    EXPECT_EQ(stream->m_lBytesLeftInStream, 16);
    EXPECT_EQ(stream->m_lSizeOfStream, 24);
    EXPECT_EQ(stream->m_lNumberOfBlocks, 3);
    EXPECT_EQ(stream->m_lDataChunks, 2);
    EXPECT_EQ(stream->m_lNumMetaChunks, 2);
    EXPECT_EQ(stream->m_lMetaChunkCounter, 2);
    EXPECT_EQ(stream->m_pMetaMem, metadata);
    EXPECT_TRUE(stream->m_bActive);
    EXPECT_FALSE(stream->m_bPreloadReady);
}

TEST(AudioStreamer, AddAudioStreamRejectsFifthConcurrentStream)
{
    TestIOStreamer streamer;
    ASSERT_TRUE(streamer.Create(8, 8, 100));

    char metadata[0x8000]{};
    streamer.SetMetaMem(metadata);
    ZBufferGroup groups[4]{};
    for (int i = 0; i < 4; ++i)
    {
        streamer.m_Streams[i].m_pUserData = &groups[i];
        ASSERT_NE(streamer.AddAudioStream(1, i * 16, 8, false, 0, 0, 0, 0, false), nullptr);
    }

    EXPECT_EQ(streamer.AddAudioStream(1, 0, 8, false, 0, 0, 0, 0, false), nullptr);
    EXPECT_EQ(streamer.m_lCurrentNumStreams, 4);
}
