#include <Glacier/Animation/StreamPacker.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

using namespace Glacier;

namespace
{
    void WriteBits(std::vector<uint8_t>& data, uint32_t& bit, uint32_t value, int count)
    {
        for (int i = 0; i < count; ++i)
        {
            if (value & (1u << i))
                data[bit >> 3] |= static_cast<uint8_t>(1u << (bit & 7));
            ++bit;
        }
    }

    // Builds the smallest valid one-channel block using a constant float.
    std::vector<uint8_t> MakeConstantFloatBlock()
    {
        std::vector<uint8_t> data(32, 0);
        data[0] = 1;
        data[1] = 0;
        data[2] = 0;
        data[3] = 0x01; // precision 0, plain float layout

        // The packed header points at a zero-sized block table.
        uint32_t bit = 80;
        WriteBits(data, bit, 1, 1); // (quantized value << 1) | constant flag
        return data;
    }
}

TEST(StreamPacker, BitPtrReadsLittleEndianBitsAcrossBytes)
{
    uint8_t data[] = { 0xB5, 0x4A, 0x00 };
    StreamPacker::BitPtr bits(data);

    EXPECT_EQ(bits.ReadBits(3), 5u);
    EXPECT_EQ(bits.ReadBits(8), 0x56u);
    EXPECT_EQ(bits.GetBytePtr(), data + 1);
}

TEST(StreamPacker, GetVectorIdsReadsCountAndAdvances)
{
    uint8_t data[] = { 7, 0xAA };
    EBoneID bone = static_cast<EBoneID>(0);

    EXPECT_EQ(StreamPacker::GetVectorIds(data, bone), data + 1);
    EXPECT_EQ(static_cast<int>(bone), 7);
}

TEST(StreamPacker, GetBlockUsesEightFrameBlocksAndRepeatsLastFrame)
{
    EXPECT_EQ(StreamPacker::GetBlock(17, 0.0f), 0);
    EXPECT_EQ(StreamPacker::GetBlock(17, 7.5f), 0);
    EXPECT_EQ(StreamPacker::GetBlock(17, 8.0f), 1);
    EXPECT_EQ(StreamPacker::GetBlock(17, 16.0f), 1);
}

TEST(StreamPacker, DePackBlockDecodesConstantFloatChannel)
{
    std::vector<uint8_t> data = MakeConstantFloatBlock();
    float decoded[104 * 9] = {};
    StreamPacker::BlockCache cache = {};
    cache.m_Data = decoded;

    StreamPacker::DePackBlock(data.data(), 8, 0, &cache);

    ASSERT_EQ(cache.m_Block, 0);
    ASSERT_EQ(cache.m_Size, 1);
    ASSERT_EQ(cache.m_Ids, reinterpret_cast<uint16_t*>(data.data() + 1));
    ASSERT_EQ(cache.m_Lps, data.data() + 3);
    for (int i = 0; i < 9; ++i)
        EXPECT_FLOAT_EQ(decoded[i], -1.0f);
}

TEST(StreamPacker, DirectAndCachedSamplingMatchForConstantFloatChannel)
{
    std::vector<uint8_t> data = MakeConstantFloatBlock();
    int16_t idToPosition[1] = { 0 };
    float direct[1] = {};
    float cached[1] = {};
    float decoded[104 * 9] = {};
    StreamPacker::BlockCache cache = {};
    cache.m_Data = decoded;

    StreamPacker::DePackBlock(data.data(), 8, 0, &cache);
    StreamPacker::DePackVector(data.data(), 8, 3.25f, idToPosition, direct, 1);
    StreamPacker::DePackBlockVector(&cache, 8, 3.25f, idToPosition, cached, 1);

    EXPECT_FLOAT_EQ(direct[0], -1.0f);
    EXPECT_FLOAT_EQ(cached[0], direct[0]);
}
