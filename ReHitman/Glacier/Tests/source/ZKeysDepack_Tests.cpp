#include <Glacier/Animation/ZKeysDepack.h>
#include <gtest/gtest.h>

#include <cmath>
#include <cstdint>
#include <vector>

using namespace Glacier;

namespace
{
    // Round-trips a V3 key table through PackKeyTables and DepackV3.
    void CheckV3RoundTrip(int lNrFrames, int lNrKeys, const int* pKeyFrames, const std::vector<float>& pValues)
    {
        ASSERT_EQ(pValues.size(), static_cast<size_t>(lNrKeys) * 3u);

        // Max frame index determines the offset/tick bit width.
        int lBitsPerOffset = 1;
        while ((1 << lBitsPerOffset) <= lNrKeys) ++lBitsPerOffset;
        int lBitsPerTime = 1;
        while ((1 << lBitsPerTime) <= lNrFrames) ++lBitsPerTime;

        const uint32_t lOffsetBytes = ZKeysDepack::OffsetTableBufferBytes(lBitsPerOffset, lNrFrames - 1);
        const uint32_t lTimeBytes = ZKeysDepack::OffsetTableBufferBytes(lBitsPerTime, lNrKeys - 1);

        std::vector<char> offsets(lOffsetBytes, 0);
        std::vector<char> times(lTimeBytes, 0);
        std::vector<float> keys(lNrKeys * 3, 0.0f);

        ZKeysDepack depack;
        ZKeysDepack::PackV3Keys(keys.data(), lNrKeys, pValues.data());
        ZKeysDepack::PackKeyTables(offsets.data(), times.data(), lBitsPerOffset, lBitsPerTime,
                                  lNrFrames, lNrKeys, pKeyFrames);

        // At each frame the depacked value must equal the nearest key <= frame.
        for (int f = 0; f < lNrFrames; ++f)
        {
            int lExpectedKey = -1;
            for (int k = 0; k < lNrKeys; ++k)
            {
                if (pKeyFrames[k] <= f)
                    lExpectedKey = k;
            }
            ASSERT_GE(lExpectedKey, 0) << "no key covers frame " << f;

            float out[3] = {};
            depack.DepackV3(out, static_cast<float>(f), static_cast<unsigned int>(lNrFrames),
                           offsets.data(), times.data(), keys.data());

            // Exact source keys must survive the pack/depack round trip.
            const bool bExactKey = (pKeyFrames[lExpectedKey] == f);
            for (int c = 0; c < 3; ++c)
            {
                const float fExpected = pValues[static_cast<size_t>(lExpectedKey) * 3 + c];
                if (bExactKey)
                    EXPECT_FLOAT_EQ(out[c], fExpected) << "frame " << f << " comp " << c;
                else
                    EXPECT_TRUE(std::isfinite(out[c]));
            }
        }
    }

    // Round-trips a Q4 key table.
    void CheckQ4RoundTrip(int lNrFrames, int lNrKeys, const int* pKeyFrames, const std::vector<float>& pQuats)
    {
        ASSERT_EQ(pQuats.size(), static_cast<size_t>(lNrKeys) * 4u);

        int lBitsPerOffset = 1;
        while ((1 << lBitsPerOffset) <= lNrKeys) ++lBitsPerOffset;
        int lBitsPerTime = 1;
        while ((1 << lBitsPerTime) <= lNrFrames) ++lBitsPerTime;

        const uint32_t lOffsetBytes = ZKeysDepack::OffsetTableBufferBytes(lBitsPerOffset, lNrFrames - 1);
        const uint32_t lTimeBytes = ZKeysDepack::OffsetTableBufferBytes(lBitsPerTime, lNrKeys - 1);

        std::vector<char> offsets(lOffsetBytes, 0);
        std::vector<char> times(lTimeBytes, 0);
        std::vector<int16_t> keys(lNrKeys * 4, 0);
        ZKeysDepack::PackQ4Keys(keys.data(), lNrKeys, pQuats.data());

        ZKeysDepack depack;
        ZKeysDepack::PackKeyTables(offsets.data(), times.data(), lBitsPerOffset, lBitsPerTime,
                                  lNrFrames, lNrKeys, pKeyFrames);

        for (int f = 0; f < lNrFrames; ++f)
        {
            int lExpectedKey = -1;
            for (int k = 0; k < lNrKeys; ++k)
            {
                if (pKeyFrames[k] <= f)
                    lExpectedKey = k;
            }
            ASSERT_GE(lExpectedKey, 0);

            float out[4] = {};
            depack.DepackQ4(out, static_cast<float>(f), static_cast<unsigned int>(lNrFrames),
                           offsets.data(), times.data(), keys.data());

            const bool bExactKey = (pKeyFrames[lExpectedKey] == f);
            for (int c = 0; c < 4; ++c)
            {
                const float fExpected = pQuats[static_cast<size_t>(lExpectedKey) * 4 + c];
                if (bExactKey)
                    EXPECT_NEAR(out[c], fExpected, 1e-3f) << "frame " << f << " comp " << c;
                else
                    EXPECT_TRUE(std::isfinite(out[c]));
            }
        }
    }
}

TEST(ZKeysDepack, ReadOffsetZeroReturnsZero)
{
    ZKeysDepack depack;
    char offsets[2] = { 4, 0 };
    EXPECT_EQ(depack.ReadOffset(offsets, 0), 0);
}

TEST(ZKeysDepack, ReadOffsetVariableWidthBitmap)
{
    // 3 bits per entry: entries {1, 5, 2, 7} packed bitwise.
    const char aData[2] = { 3, 0 };
    char offsets[8] = { 3, 0, 0, 0, 0, 0, 0, 0 };
    ZKeysDepack::WriteOffset(offsets, 3, 1, 1);
    ZKeysDepack::WriteOffset(offsets, 3, 2, 5);
    ZKeysDepack::WriteOffset(offsets, 3, 3, 2);
    ZKeysDepack::WriteOffset(offsets, 3, 4, 7);

    EXPECT_EQ(static_cast<unsigned char>(offsets[0]), 3);
    EXPECT_EQ(static_cast<unsigned char>(offsets[1]), 0x35); // 001 101 01
    EXPECT_EQ(static_cast<unsigned char>(offsets[2]), 0x70); // 0 111 0000

    ZKeysDepack depack;
    EXPECT_EQ(depack.ReadOffset(offsets, 1), 1);
    EXPECT_EQ(depack.ReadOffset(offsets, 2), 5);
    EXPECT_EQ(depack.ReadOffset(offsets, 3), 2);
    EXPECT_EQ(depack.ReadOffset(offsets, 4), 7);
}

TEST(ZKeysDepack, PackQuatKeyRoundTrip)
{
    const float aQuat[4] = { 0.5f, -0.5f, 0.70710678f, 0.5f };
    int16_t aKeys[4] = {};
    float aBack[4] = {};

    ZKeysDepack::PackQuatKey(aKeys, aQuat);
    ZKeysDepack depack;
    depack.DepackQuatKey(aBack, aKeys);

    for (int c = 0; c < 4; ++c)
        EXPECT_NEAR(aBack[c], aQuat[c], 1e-3f) << "comp " << c;
}

TEST(ZKeysDepack, V3RoundTripTwoKeys)
{
    const int aFrames[2] = { 0, 5 };
    const std::vector<float> aValues = {
        1.0f, 2.0f, 3.0f,
        4.0f, 5.0f, 6.0f,
    };
    CheckV3RoundTrip(6, 2, aFrames, aValues);
}

TEST(ZKeysDepack, V3RoundTripMultipleKeys)
{
    const int aFrames[3] = { 0, 3, 7 };
    const std::vector<float> aValues = {
        -1.5f, 0.0f, 2.5f,
         3.0f, 4.0f, 5.0f,
         6.0f, 7.0f, 8.0f,
    };
    CheckV3RoundTrip(8, 3, aFrames, aValues);
}

TEST(ZKeysDepack, V3RoundTripSingleKey)
{
    const int aFrames[1] = { 0 };
    const std::vector<float> aValues = { 10.0f, 20.0f, 30.0f };
    CheckV3RoundTrip(1, 1, aFrames, aValues);
}

TEST(ZKeysDepack, Q4RoundTripTwoKeys)
{
    const int aFrames[2] = { 0, 4 };
    const std::vector<float> aQuats = {
        0.0f, 0.0f, 0.0f, 1.0f,       // identity: {i, j, k, real}
        0.70710678f, 0.0f, 0.0f, 0.70710678f,
    };
    CheckQ4RoundTrip(5, 2, aFrames, aQuats);
}

TEST(ZKeysDepack, Q4RoundTripMultipleKeys)
{
    const int aFrames[3] = { 0, 2, 6 };
    const std::vector<float> aQuats = {
        0.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 0.70710678f, 0.0f, 0.70710678f,
        0.0f, 0.0f, 0.70710678f, 0.70710678f,
    };
    CheckQ4RoundTrip(7, 3, aFrames, aQuats);
}

TEST(ZKeysDepack, V3InterpolatesBetweenKeys)
{
    // Two keys on frames 0 and 10; at frame 5 the V3 should interpolate to the midpoint.
    const int aFrames[2] = { 0, 10 };
    const std::vector<float> aValues = {
        0.0f, 0.0f, 0.0f,
        10.0f, 10.0f, 10.0f,
    };
    const int lNrFrames = 11;
    int lBitsPerOffset = 1;
    while ((1 << lBitsPerOffset) <= 2) ++lBitsPerOffset;
    int lBitsPerTime = 1;
    while ((1 << lBitsPerTime) <= lNrFrames) ++lBitsPerTime;

    const uint32_t lOffsetBytes = ZKeysDepack::OffsetTableBufferBytes(lBitsPerOffset, lNrFrames - 1);
    const uint32_t lTimeBytes = ZKeysDepack::OffsetTableBufferBytes(lBitsPerTime, 1);

    std::vector<char> offsets(lOffsetBytes, 0);
    std::vector<char> times(lTimeBytes, 0);
    std::vector<float> keys(2 * 3, 0.0f);

    ZKeysDepack depack;
    ZKeysDepack::PackV3Keys(keys.data(), 2, aValues.data());
    ZKeysDepack::PackKeyTables(offsets.data(), times.data(), lBitsPerOffset, lBitsPerTime,
                              lNrFrames, 2, aFrames);

    float out[3] = {};
    depack.DepackV3(out, 5.0f, lNrFrames, offsets.data(), times.data(), keys.data());

    EXPECT_NEAR(out[0], 5.0f, 1e-3f);
    EXPECT_NEAR(out[1], 5.0f, 1e-3f);
    EXPECT_NEAR(out[2], 5.0f, 1e-3f);
}
