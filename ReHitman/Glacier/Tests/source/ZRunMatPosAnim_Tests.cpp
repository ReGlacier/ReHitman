#include <Glacier/Animation/ZRunMatPosAnim.h>
#include <Glacier/ZSTL/CHUNKFILE.h>
#include <Glacier/Serializer/ZPackedOutput.h>
#include <Glacier/Serializer/ZPackedInput.h>
#include <Glacier/Animation/ZKeysDepack.h>
#include <Glacier/ZUniMemory.h>
#include <Tests/EngineFixture.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstring>
#include <vector>

using namespace Glacier;

namespace
{
    struct MemoryOutputStream final : IOutputStream
    {
        std::vector<char> Bytes;

        uint32_t Write(const void* pAddr, const uint32_t lSize) override
        {
            const auto* pBytes = static_cast<const char*>(pAddr);
            Bytes.insert(Bytes.end(), pBytes, pBytes + lSize);
            return lSize;
        }
    };

    struct MemoryInputStream final : ZInputStreamBase
    {
        explicit MemoryInputStream(const std::vector<char>& bytes)
            : Bytes(bytes)
        {
            m_ChangeEndianness = false;
        }

        uint32_t ReadRaw(char* pAddress, const uint32_t lSize) override
        {
            const uint32_t lRemaining = static_cast<uint32_t>(Bytes.size() - Offset);
            const uint32_t lReadSize = std::min(lSize, lRemaining);
            if (lReadSize)
                std::memcpy(pAddress, Bytes.data() + Offset, lReadSize);
            Offset += lReadSize;
            return lReadSize;
        }

        uint32_t ReadChangeEndianness(char* pAddress, const uint32_t lSize, const uint32_t lMask) override
        {
            uint32_t lReadSize = 0;
            while (lReadSize < lSize && Offset < Bytes.size())
            {
                pAddress[lReadSize ^ lMask] = Bytes[Offset++];
                ++lReadSize;
            }
            return lReadSize;
        }

        const std::vector<char>& Bytes;
        size_t Offset{ 0 };
    };

    CHUNKFILE MakeFlatMatPosChunk(int lFrameCount, int lFrameStart, int lFrameEnd)
    {
        CHUNKFILE chunk{};
        chunk.m_lTotalSize = 8 + 12;
        auto* pData = reinterpret_cast<int*>(&chunk.type);
        pData[0] = lFrameCount;
        pData[1] = lFrameStart;
        pData[2] = lFrameEnd;
        return chunk;
    }
}

TEST(ZRunMatPosAnim, ConstructorInitializesDefaults)
{
    ZRunMatPosAnim animation;

    EXPECT_EQ(animation.m_fStTime.secs, 0);
    EXPECT_FLOAT_EQ(animation.FPS(), 0.0f);
    EXPECT_FALSE(animation.m_bLoop);
    EXPECT_EQ(animation.m_pChunkFile, nullptr);
    EXPECT_FLOAT_EQ(animation.GetStart(), 0.0f);
    EXPECT_FLOAT_EQ(animation.GetEnd(), 0.0f);
}

TEST(ZRunMatPosAnim, SetStartTimePreservesRawFixedPointTicks)
{
    ZRunMatPosAnim animation;
    TIMETYPE start(1.5f);

    animation.SetStartTime(start);

    EXPECT_EQ(animation.m_fStTime.secs, 1536);
}

TEST(ZRunMatPosAnim, CreateStoresParametersAndClampsFrames)
{
    CHUNKFILE chunk = MakeFlatMatPosChunk(12, 2, 9);
    ZRunMatPosAnim animation;

    animation.Create(&chunk, TIMETYPE(2.25f), true, 24.0f);

    EXPECT_EQ(animation.m_pChunkFile, &chunk);
    EXPECT_EQ(animation.m_fStTime.secs, 2304);
    EXPECT_TRUE(animation.m_bLoop);
    EXPECT_FLOAT_EQ(animation.FPS(), 24.0f);
    EXPECT_FLOAT_EQ(animation.GetStart(), 2.0f);
    EXPECT_FLOAT_EQ(animation.GetEnd(), 9.0f);
    EXPECT_FLOAT_EQ(animation.GetAnimEndTime(), 9.0f);
}

TEST(ZRunMatPosAnim, FrameSettersClampToChunkRange)
{
    CHUNKFILE chunk = MakeFlatMatPosChunk(12, 2, 9);
    ZRunMatPosAnim animation;
    animation.SetChunkFile(&chunk);

    animation.SetFrameStart(-100);
    EXPECT_FLOAT_EQ(animation.GetStart(), 2.0f);
    animation.SetFrameStart(100);
    EXPECT_FLOAT_EQ(animation.GetStart(), 9.0f);

    animation.SetFrameEnd(-100);
    EXPECT_FLOAT_EQ(animation.GetEnd(), 9.0f);
    animation.SetFrameEnd(5);
    EXPECT_FLOAT_EQ(animation.GetEnd(), 5.0f);
    animation.SetFrameEnd(100);
    EXPECT_FLOAT_EQ(animation.GetEnd(), 9.0f);
}

TEST(ZRunMatPosAnim, PackedSerializationRoundTripsAllFieldsAndRawTimeTicks)
{
    ZRunMatPosAnim source;
    source.SetStartTime(TIMETYPE(1.5f));
    source.m_fFPS = 30.0f;
    source.m_bLoop = true;
    source.m_fFrameStart = 2.25f;
    source.m_fFrameEnd = 8.75f;

    MemoryOutputStream memory;
    {
        ZPackedOutput output(memory, nullptr, ISerializerStream::TYPE_Tags,
                             ISerializerStream::CONTENT_SimpleRepack, false);
        source.SaveObject(output);
        output.End();
    }

    MemoryInputStream memoryInput(memory.Bytes);
    ZPackedInput input(&memoryInput);
    ZRunMatPosAnim loaded;
    loaded.LoadObject(input);
    input.End();

    EXPECT_EQ(loaded.m_fStTime.secs, 1536); // raw fixed-point ticks, not truncated seconds
    EXPECT_FLOAT_EQ(loaded.m_fFPS, 30.0f);
    EXPECT_TRUE(loaded.m_bLoop);
    EXPECT_FLOAT_EQ(loaded.m_fFrameStart, 2.25f);
    EXPECT_FLOAT_EQ(loaded.m_fFrameEnd, 8.75f);
}

TEST(ZRunMatPosAnim, LoadObjectClampsSerializedFramesWhenChunkIsPresent)
{
    ZRunMatPosAnim source;
    source.m_fFrameStart = -20.0f;
    source.m_fFrameEnd = 100.0f;

    MemoryOutputStream memory;
    {
        ZPackedOutput output(memory, nullptr, ISerializerStream::TYPE_Tags,
                             ISerializerStream::CONTENT_SimpleRepack, false);
        source.SaveObject(output);
        output.End();
    }

    CHUNKFILE chunk = MakeFlatMatPosChunk(12, 2, 9);
    ZRunMatPosAnim loaded;
    loaded.SetChunkFile(&chunk);
    MemoryInputStream memoryInput(memory.Bytes);
    ZPackedInput input(&memoryInput);
    loaded.LoadObject(input);
    input.End();

    EXPECT_FLOAT_EQ(loaded.GetStart(), 2.0f);
    EXPECT_FLOAT_EQ(loaded.GetEnd(), 9.0f);
}

TEST(ZRunMatPosAnim, RunWithoutChunkReturnsIdentityAndZeroPosition)
{
    ZRunMatPosAnim animation;
    float matrix[9] = { 99.0f, 99.0f, 99.0f, 99.0f, 99.0f, 99.0f, 99.0f, 99.0f, 99.0f };
    float position[3] = { 99.0f, 99.0f, 99.0f };

    EXPECT_FLOAT_EQ(animation.Run(TIMETYPE(1), matrix, position, 0.0f), 0.0f);
    EXPECT_EQ(matrix[2], 1.0f);
    EXPECT_EQ(matrix[4], 1.0f);
    EXPECT_EQ(matrix[6], 1.0f);
    EXPECT_EQ(position[0], 0.0f);
    EXPECT_EQ(position[1], 0.0f);
    EXPECT_EQ(position[2], 0.0f);
}

TEST(ZRunMatPosAnim, GetAnimEndTimeAssertsWithoutChunk)
{
    ZRunMatPosAnim animation;
    EXPECT_THROW(animation.GetAnimEndTime(), std::exception);
}

class ZRunMatPosAnimEngineTest : public Tests::EngineFixture
{
};

TEST_F(ZRunMatPosAnimEngineTest, RunDepacksSingleMatPosKeyFromStaticBuffer)
{
    // Layout pData[4..9]: Q offsets, Q times, Q keys, V offsets, V times, V keys.
    constexpr int kQOffsets = 0;
    constexpr int kQTimes = 4;
    constexpr int kQKeys = 8;
    constexpr int kVOffsets = 16;
    constexpr int kVTimes = 20;
    constexpr int kVKeys = 24;

    auto* pStaticBuffer = static_cast<uint8_t*>(ZUniMemory::Allocate(64));
    std::memset(pStaticBuffer, 0, 64);

    // Width bytes are present even though frame/key zero is implicit.
    pStaticBuffer[kQOffsets] = 1;
    pStaticBuffer[kQTimes] = 1;
    pStaticBuffer[kVOffsets] = 1;
    pStaticBuffer[kVTimes] = 1;

    const float identityQuat[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    ZKeysDepack::PackQuatKey(reinterpret_cast<int16_t*>(pStaticBuffer + kQKeys), identityQuat);
    const float position[3] = { 4.0f, 5.0f, 6.0f };
    std::memcpy(pStaticBuffer + kVKeys, position, sizeof(position));

    EngineData().m_pStaticBuffer = pStaticBuffer;
    EngineData().m_lStaticBufferLength = 64;

    struct MatPosChunk
    {
        CHUNKFILE Chunk;
        int ExtraData[7];
    } chunk{};
    chunk.Chunk.m_lTotalSize = 8 + 40;
    int* pData = reinterpret_cast<int*>(&chunk.Chunk.type);
    pData[0] = 1; // one frame
    pData[1] = 0; // first frame
    pData[2] = 0; // last frame
    pData[3] = 1; // frame divisor
    pData[4] = kQOffsets;
    pData[5] = kQTimes;
    pData[6] = kQKeys;
    pData[7] = kVOffsets;
    pData[8] = kVTimes;
    pData[9] = kVKeys;

    ZRunMatPosAnim animation;
    animation.Create(&chunk.Chunk, TIMETYPE(0), false, 30.0f);

    float matrix[9] = {};
    float outputPosition[3] = {};
    const float result = animation.Run(TIMETYPE(0), matrix, outputPosition, 0.0f);

    EXPECT_FLOAT_EQ(result, 0.0f);
    EXPECT_NEAR(outputPosition[0], 4.0f, 1e-5f);
    EXPECT_NEAR(outputPosition[1], 5.0f, 1e-5f);
    EXPECT_NEAR(outputPosition[2], 6.0f, 1e-5f);
    EXPECT_NEAR(matrix[2], 1.0f, 1e-3f);
    EXPECT_NEAR(matrix[4], 1.0f, 1e-3f);
    EXPECT_NEAR(matrix[6], 1.0f, 1e-3f);

    // Prevent ZEngineDataBase from owning/freeing the test buffer twice.
    EngineData().m_pStaticBuffer = nullptr;
    EngineData().m_lStaticBufferLength = 0;
    ZUniMemory::Free(pStaticBuffer);
}
