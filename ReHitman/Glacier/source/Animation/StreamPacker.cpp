#include <Glacier/Animation/StreamPacker.h>
#include <Glacier/ZUniAssert.h>
#include <Glacier/ZSTL/ZMath.h>
#include <numbers>
#include <cmath>


namespace
{
    uint16_t ReadU16(const uint8_t* p)
    {
        return static_cast<uint16_t>(p[0]) | (static_cast<uint16_t>(p[1]) << 8);
    }

    int32_t SignExtend(uint32_t value, int bitCount)
    {
        if (bitCount == 0)
            return 0;

        return static_cast<int32_t>(value << (32 - bitCount)) >> (32 - bitCount);
    }
}


namespace Glacier
{
    struct SLayoutInfo
    {
        uint8_t vectorComponents;
        uint8_t hasQuaternion;
    };
    RE_VERIFY_SIZE(SLayoutInfo, 2);

    struct SDataInfo
    {
        float f_Low;
        float f_Range;
        uint8_t i_Floats;
        uint8_t i_Quats;
    };
    RE_VERIFY_SIZE(SDataInfo, 12);

    // Quantization normalization factors: 1 / (2^n - 1).
    constexpr float g_afLookup[16] =
    {
        1.0f, 1.0f, 0.33333334f, 0.14285715f, 0.06666667f, 0.032258064f, 0.015873017f, 0.0078740157f,
        0.0039215689f, 0.0019569471f, 0.00097751711f, 0.00048851978f, 0.00024420026f, 0.00012208521f,
        0.000061038882f, 0.000030518509f
    };

    // Per-layout decode parameters (range/low + number of vector/quat components).
    constexpr SDataInfo g_DataInfo[16] =
    {
        /* 0 */ { -std::numbers::pi_v<float>, 2.0f * std::numbers::pi_v<float>, 1, 0 }, // Angle
        /* 1 */ { -1.0f, 2.0f, 1, 0 },                                                // Float
        /* 2 */ { 0.0f, 0.0f, 1, 0 },                                                 // Generic float (read from stream)
        /* 3 */ { 0.0f, 1.0f, 0, 1 },                                                 // Quaternion
        /* 4 */ { -10.0f, 20.0f, 3, 1 },                                              // Vector + Quaternion
        /* 5 */ { -100.0f, 200.0f, 3, 1 },
        /* 6 */ { -1000.0f, 2000.0f, 3, 1 },
        /* 7 */ { -10000.0f, 20000.0f, 3, 1 },
        /* 8 */ { -100000.0f, 200000.0f, 3, 1 },
    };

    // DePackBlockVector output layout per stream layout index.
    constexpr SLayoutInfo g_LayoutTable[16] =
    {
        /* 0 */ { 1, 0 }, // Angle (1 float, no quaternion)
        /* 1 */ { 1, 0 }, // Float
        /* 2 */ { 1, 0 }, // Generic float
        /* 3 */ { 0, 1 }, // Quaternion
        /* 4 */ { 3, 1 }, // Vector + Quaternion
        /* 5 */ { 3, 1 },
        /* 6 */ { 3, 1 },
        /* 7 */ { 3, 1 },
        /* 8 */ { 3, 1 },
        /* 9 */ { 0, 0 },
    };

    StreamPacker::BitPtr::BitPtr(uint8_t* ptr)
        : base(ptr)
        , bit(0)
    {
    }

    StreamPacker::BitPtr::BitPtr(const BitPtr& copy)
        : base(copy.base)
        , bit(copy.bit)
    {
    }

    uint8_t* StreamPacker::BitPtr::GetBytePtr()
    {
        return &base[bit >> 3];
    }

    void StreamPacker::DePackVector(uint8_t* data, int samples, float frame2, int16_t* pId2Pos, float* pSamples, int bailAt)
    {
        const int numBlocks = (samples + 7) >> 3;

        int intframe = static_cast<int>(std::floor(frame2));
        ZASSERT(intframe <= samples - 1);

        if (intframe == samples - 1)
            --intframe;

        const int subFrameIndex = intframe & 7;
        const int blockIndex = intframe >> 3;
        const float t = frame2 - static_cast<float>(intframe);

        int blockSamples = 8;
        if (blockIndex == numBlocks - 1)
            blockSamples = samples - 8 * blockIndex;

        const int count = data[0];

        int headerBits = 16 * count + 8 + 8 * count;
        const uint8_t* pLps = &data[(16 * count + 8) >> 3];

        if (headerBits & 0xF)
            headerBits += 16 - (headerBits & 0xF);

        const int lMidSize = ReadU16(&data[(headerBits >> 3) + 4]);
        BitPtr bitPtr(data);
        bitPtr.bit = headerBits + 48;
        const int blockTableBits = 16 * ReadU16(&data[(headerBits >> 3) + 2]) + headerBits + 48;
        const uint8_t* pBlockTable = &data[blockTableBits >> 3];

        int firstBits = 2 * ReadU16(&pBlockTable[2 * blockIndex]);
        int lastBits = 2 * ReadU16(&pBlockTable[2 * blockIndex + 2]);
        if (blockIndex == numBlocks - 1)
            lastBits = firstBits;

        const int streamsBits = blockTableBits + 16 * numBlocks + 16;
        BitPtr firstStream(data);
        firstStream.bit = streamsBits + 8 * firstBits;
        BitPtr lastStream(data);
        lastStream.bit = streamsBits + 8 * lastBits;
        BitPtr midStream(data);
        midStream.bit = streamsBits + 8 * (firstBits + lMidSize);

        float scratch[16] = {};

        for (int i = 0; i < count; ++i)
        {
            const uint16_t boneId = ReadU16(&data[1 + 2 * i]);
            if (static_cast<int>(boneId) >= bailAt)
                return;

            const uint8_t layoutByte = pLps[i];
            const int layout = layoutByte & 0xF;
            const int precision = layoutByte >> 4;
            const bool isEuler = (layout == 0);

            const int16_t targetOffset = pId2Pos[boneId];
            float* pDst = (targetOffset == -1) ? scratch : &pSamples[targetOffset];

            SDataInfo info = g_DataInfo[layout];
            int quatPrecision = precision;

            if (layout == 2)
            {
                const int16_t low = static_cast<int16_t>(bitPtr.ReadBits(16));
                const int16_t high = static_cast<int16_t>(bitPtr.ReadBits(16));
                info.f_Low = static_cast<float>(low);
                info.f_Range = static_cast<float>(high - low);
            }
            else if (layout >= 4)
            {
                quatPrecision = 12;
            }

            if (info.i_Quats)
            {
                uint16_t quatScratch[8] = {};

                const uint32_t q = bitPtr.ReadBits(4);
                const uint16_t compIndex = static_cast<uint16_t>(q >> 1);
                quatScratch[0] = compIndex;
                quatScratch[4] = compIndex;

                if (!(q & 1))
                {
                    const uint32_t midStart = midStream.bit;

                    uint16_t first = static_cast<uint16_t>(firstStream.ReadBits(3));
                    uint16_t last = static_cast<uint16_t>(lastStream.ReadBits(3));

                    if (subFrameIndex > 0)
                    {
                        midStream.bit = midStart + 3 * subFrameIndex - 3;
                        first = static_cast<uint16_t>(midStream.ReadBits(3));
                    }

                    if (subFrameIndex < 7)
                    {
                        last = static_cast<uint16_t>(midStream.ReadBits(3));
                    }

                    midStream.bit = midStart + 3 * (blockSamples - 1);

                    quatScratch[0] = first;
                    quatScratch[4] = last;
                }

                for (int c = 0; c < 3; ++c)
                {
                    const uint32_t v = bitPtr.ReadBits(quatPrecision + 1);
                    const uint32_t base = v >> 1;
                    quatScratch[1 + c] = static_cast<uint16_t>(base);
                    quatScratch[5 + c] = static_cast<uint16_t>(base);

                    if (!(v & 1))
                    {
                        const int bitCount = bitPtr.ReadBits(4) + 1;
                        int32_t first = static_cast<int32_t>(base) + static_cast<int32_t>(firstStream.ReadBits(bitCount));
                        int32_t last = static_cast<int32_t>(base) + static_cast<int32_t>(lastStream.ReadBits(bitCount));

                        const int midBitCount = midStream.ReadBits(4);
                        for (int k = 0; k < subFrameIndex; ++k)
                            first += SignExtend(midStream.ReadBits(midBitCount), midBitCount);

                        if (subFrameIndex < 7)
                        {
                            last = first + SignExtend(midStream.ReadBits(midBitCount), midBitCount);
                            midStream.Skip(midBitCount * (blockSamples - subFrameIndex - 2));
                        }
                        else
                        {
                            midStream.Skip(midBitCount * (blockSamples - subFrameIndex - 1));
                        }

                        quatScratch[1 + c] = static_cast<uint16_t>(first);
                        quatScratch[5 + c] = static_cast<uint16_t>(last);
                    }
                }

                float quats[8] = {};
                const float quatScale = 1.42f / static_cast<float>(1 << quatPrecision);

                for (int q = 0; q < 2; ++q)
                {
                    const uint16_t idx = quatScratch[q * 4];
                    const int n = idx >> 1;
                    const int sign = (idx & 1) ? 1 : -1;

                    const int i0 = (n < 1) ? 1 : 0;
                    const int i1 = (n < 2) ? 2 : 1;
                    const int i2 = (n < 3) ? 3 : 2;

                    const float c0 = static_cast<float>(quatScratch[q * 4 + 1]) * quatScale - 0.71f;
                    const float c1 = static_cast<float>(quatScratch[q * 4 + 2]) * quatScale - 0.71f;
                    const float c2 = static_cast<float>(quatScratch[q * 4 + 3]) * quatScale - 0.71f;

                    quats[q * 4 + i0] = c0;
                    quats[q * 4 + i1] = c1;
                    quats[q * 4 + i2] = c2;

                    const float remainder = 1.0f - (c0 * c0 + c1 * c1 + c2 * c2);
                    if (remainder <= 0.0f)
                    {
                        quats[q * 4 + 0] = 0.0f;
                        quats[q * 4 + 1] = 0.0f;
                        quats[q * 4 + 2] = 0.0f;
                        quats[q * 4 + 3] = 1.0f;
                    }
                    else
                    {
                        quats[q * 4 + n] = std::sqrt(remainder) * static_cast<float>(sign);
                    }
                }

                qpul(pDst, &quats[0], &quats[4], t);
                pDst += 9;
            }

            if (info.i_Floats)
            {
                ZASSERT(precision <= 15);

                const float scale = g_afLookup[precision] * info.f_Range;

                for (int c = 0; c < info.i_Floats; ++c)
                {
                    const uint32_t v = bitPtr.ReadBits(precision + 1);
                    const uint32_t base = v >> 1;

                    if (v & 1)
                    {
                        *pDst++ = static_cast<float>(base) * scale + info.f_Low;
                    }
                    else
                    {
                        const int bitCount = bitPtr.ReadBits(4) + 1;
                        int32_t first = static_cast<int32_t>(base) + static_cast<int32_t>(firstStream.ReadBits(bitCount));
                        int32_t last = static_cast<int32_t>(base) + static_cast<int32_t>(lastStream.ReadBits(bitCount));

                        const int midBitCount = midStream.ReadBits(4);
                        for (int k = 0; k < subFrameIndex; ++k)
                            first += SignExtend(midStream.ReadBits(midBitCount), midBitCount);

                        if (subFrameIndex < 7)
                        {
                            last = first + SignExtend(midStream.ReadBits(midBitCount), midBitCount);
                            midStream.Skip(midBitCount * (blockSamples - subFrameIndex - 2));
                        }
                        else
                        {
                            midStream.Skip(midBitCount * (blockSamples - subFrameIndex - 1));
                        }

                        float result;
                        if (isEuler && std::fabs(static_cast<float>(first - last) * scale) > info.f_Range * 0.5f)
                        {
                            float a = static_cast<float>(first) * scale;
                            float b = static_cast<float>(last) * scale;

                            if (a >= info.f_Range * 0.5f)
                                b += info.f_Range;
                            else
                                a += info.f_Range;

                            result = a * (1.0f - t) + b * t;

                            if (result > info.f_Range)
                                result -= info.f_Range;
                        }
                        else
                        {
                            result = (static_cast<float>(first) * (1.0f - t) + static_cast<float>(last) * t) * scale;
                        }

                        *pDst++ = result + info.f_Low;
                    }
                }
            }

            if (info.i_Quats && info.i_Floats)
            {
                pDst[-8] = pDst[-3];
                pDst[-7] = pDst[-2];
                pDst[-6] = pDst[-1];
            }
        }
    }

    void StreamPacker::DePackBlockVector(StreamPacker::BlockCache* pBlockCache, int samples, float frame2, int16_t* pId2Pos, float* pSamples, int bailAt)
    {
        constexpr float PI = std::numbers::pi_v<float>;
        constexpr float TWO_PI = std::numbers::pi_v<float> * 2.0f;

        int currentFrame = static_cast<int>(std::floor(frame2));

        if (currentFrame > samples - 1)
        {
            ZASSERT(false); // Frame out of bounds
        }

        if (currentFrame == samples - 1)
        {
            currentFrame--;
        }

        const int subFrameIndex = currentFrame & 7;
        const int blockIndex = currentFrame >> 3;

        const float t = frame2 - static_cast<float>(currentFrame);
        const float invT = 1.0f - t;

        if (blockIndex != pBlockCache->m_Block)
        {
            ZASSERT(false); // Block not in cache
        }

        float* pSrcData = pBlockCache->m_Data;
        if (pBlockCache->m_Size <= 0)
            return;

        // Iterate over cache and lookup for entry
        for (int i = 0; i < pBlockCache->m_Size; ++i)
        {
            uint16_t lBoneId = pBlockCache->m_Ids[i];

            if (static_cast<int>(lBoneId) >= bailAt)
                break;

            uint8_t layoutIndex = pBlockCache->m_Lps[i] & 0x0F;
            SLayoutInfo layout = g_LayoutTable[layoutIndex];

            int16_t targetOffset = pId2Pos[lBoneId];
            bool isEulerAngle = (layoutIndex == 0);

            // Element skipped
            if (targetOffset == -1)
            {
                if (layout.hasQuaternion)
                {
                    pSrcData += 36;
                }
                pSrcData += 9 * layout.vectorComponents;
                continue;
            }

            // Interpolation
            float* pDstSamples = &pSamples[targetOffset];

            // is it quat?
            if (layout.hasQuaternion)
            {
                const float* q0 = &pSrcData[4 * subFrameIndex];
                const float* q1 = &pSrcData[4 * subFrameIndex + 4];

                qpul(pDstSamples, q0, q1, t);

                pDstSamples += 9;
                pSrcData += 36;
            }

            // is that vector comp?
            for (int c = 0; c < layout.vectorComponents; ++c)
            {
                float val0 = pSrcData[subFrameIndex];
                float val1 = pSrcData[subFrameIndex + 1];
                float result;

                // Lerp with wrap-around 2*PI
                if (isEulerAngle && std::fabs(val0 - val1) > PI)
                {
                    if (val0 >= 0.0f)
                        val1 += TWO_PI;
                    else
                        val0 += TWO_PI;

                    result = val0 * invT + val1 * t;

                    if (result > TWO_PI)
                        result -= TWO_PI;
                }
                else
                {
                    // Generic Lerp
                    result = val0 * invT + val1 * t;
                }

                *pDstSamples = result;

                pDstSamples += 1;
                pSrcData += 9;
            }

            // Make transform
            if (layout.hasQuaternion && layout.vectorComponents != 0)
            {
                pDstSamples[-8] = pDstSamples[-3];
                pDstSamples[-7] = pDstSamples[-2];
                pDstSamples[-6] = pDstSamples[-1];
            }
        }
    }

    uint8_t* StreamPacker::GetVectorIds(uint8_t* pBuffer, EBoneID& eBone)
    {
        eBone = static_cast<EBoneID>(*pBuffer);
        return pBuffer + 1;
    }

    int32_t StreamPacker::GetBlock(int32_t lDuration, float fFrame)
    {
        auto lFrame2 = static_cast<int32_t>(floor(fFrame));
        if (lFrame2 == lDuration - 1)
        {
            --lFrame2;
        }

        return lFrame2 >> 3;
    }

    void StreamPacker::DePackBlock(uint8_t* data, int samples, int lBlock, StreamPacker::BlockCache* pBlockCache)
    {
        const int numBlocks = (samples + 7) >> 3;
        int blockSamples = 8;
        if (lBlock == numBlocks - 1)
            blockSamples = samples - 8 * lBlock;

        const int count = data[0];
        pBlockCache->m_Size = static_cast<int16_t>(count);
        pBlockCache->m_Block = static_cast<int16_t>(lBlock);
        pBlockCache->m_Ids = reinterpret_cast<uint16_t*>(data + 1);

        int headerBits = 16 * count + 8 + 8 * count;
        pBlockCache->m_Lps = &data[(16 * count + 8) >> 3];
        const uint8_t* pLps = &data[(16 * count + 8) >> 3];

        if (headerBits & 0xF)
            headerBits += 16 - (headerBits & 0xF);

        const int lMidSize = ReadU16(&data[(headerBits >> 3) + 4]);
        BitPtr bitPtr(data);
        bitPtr.bit = headerBits + 48;
        const int blockTableBits = 16 * ReadU16(&data[(headerBits >> 3) + 2]) + headerBits + 48;
        const uint8_t* pBlockTable = &data[blockTableBits >> 3];

        int firstBits = 2 * ReadU16(&pBlockTable[2 * lBlock]);
        int lastBits = 2 * ReadU16(&pBlockTable[2 * lBlock + 2]);
        if (lBlock == numBlocks - 1)
            lastBits = firstBits;

        const int streamsBits = blockTableBits + 16 * numBlocks + 16;
        BitPtr firstStream(data);
        firstStream.bit = streamsBits + 8 * firstBits;
        BitPtr lastStream(data);
        lastStream.bit = streamsBits + 8 * lastBits;
        BitPtr midStream(data);
        midStream.bit = streamsBits + 8 * (firstBits + lMidSize);

        float* pData = pBlockCache->m_Data;
        float* const pDataStart = pData;

        for (int i = 0; i < count; ++i)
        {
            const int layout = pLps[i] & 0xF;
            const int precision = pLps[i] >> 4;

            SDataInfo info = g_DataInfo[layout];
            int quatPrecision = precision;

            if (layout == 2)
            {
                const int16_t low = static_cast<int16_t>(bitPtr.ReadBits(16));
                const int16_t high = static_cast<int16_t>(bitPtr.ReadBits(16));
                info.f_Low = static_cast<float>(low);
                info.f_Range = static_cast<float>(high - low);
            }
            else if (layout >= 4)
            {
                quatPrecision = 12;
            }

            if (info.i_Quats)
            {
                uint16_t quatIndex[9] = {};
                const uint32_t q = bitPtr.ReadBits(4);

                if (q & 1)
                {
                    const uint16_t compIndex = static_cast<uint16_t>(q >> 1);
                    for (int m = 0; m < 9; ++m)
                        quatIndex[m] = compIndex;
                }
                else
                {
                    quatIndex[0] = static_cast<uint16_t>(firstStream.ReadBits(3));
                    quatIndex[8] = static_cast<uint16_t>(lastStream.ReadBits(3));
                    for (int m = 0; m < blockSamples - 1; ++m)
                        quatIndex[m + 1] = static_cast<uint16_t>(midStream.ReadBits(3));
                }

                int16_t quatComponents[27] = {};
                for (int c = 0; c < 3; ++c)
                {
                    const uint32_t v = bitPtr.ReadBits(quatPrecision + 1);
                    const bool isConst = (v & 1) != 0;
                    const uint32_t base = v >> 1;

                    if (isConst)
                    {
                        for (int m = 0; m < 9; ++m)
                            quatComponents[3 * m + c] = static_cast<int16_t>(base);
                    }
                    else
                    {
                        const int bitCount = bitPtr.ReadBits(4) + 1;
                        const uint32_t firstDelta = firstStream.ReadBits(bitCount);
                        const uint32_t lastDelta = lastStream.ReadBits(bitCount);

                        quatComponents[c] = static_cast<int16_t>(base + firstDelta);
                        quatComponents[24 + c] = static_cast<int16_t>(base + lastDelta);

                        const int midBitCount = midStream.ReadBits(4);
                        for (int m = 0; m < blockSamples - 1; ++m)
                        {
                            const uint32_t delta = midStream.ReadBits(midBitCount);
                            quatComponents[3 * (m + 1) + c] = static_cast<int16_t>(
                                quatComponents[3 * m + c] + SignExtend(delta, midBitCount));
                        }
                    }
                }

                const int quatCount = (blockSamples == 8) ? 9 : blockSamples;
                const float quatScale = 1.42f / static_cast<float>(1 << quatPrecision);

                for (int m = 0; m < quatCount; ++m)
                {
                    const uint16_t idx = quatIndex[m];
                    const int n = idx >> 1;
                    const int sign = (idx & 1) ? 1 : -1;

                    const int i0 = (n < 1) ? 1 : 0;
                    const int i1 = (n < 2) ? 2 : 1;
                    const int i2 = (n < 3) ? 3 : 2;

                    const float c0 = static_cast<float>(quatComponents[3 * m + 0]) * quatScale - 0.71f;
                    const float c1 = static_cast<float>(quatComponents[3 * m + 1]) * quatScale - 0.71f;
                    const float c2 = static_cast<float>(quatComponents[3 * m + 2]) * quatScale - 0.71f;

                    pData[i0] = c0;
                    pData[i1] = c1;
                    pData[i2] = c2;

                    const float remainder = 1.0f - (c0 * c0 + c1 * c1 + c2 * c2);
                    if (remainder <= 0.0f)
                    {
                        pData[0] = 0.0f;
                        pData[1] = 0.0f;
                        pData[2] = 0.0f;
                        pData[3] = 1.0f;
                    }
                    else
                    {
                        pData[n] = std::sqrt(remainder) * static_cast<float>(sign);
                    }

                    pData += 4;
                }

                pData += 4 * (9 - quatCount);
            }

            if (info.i_Floats)
            {
                ZASSERT(precision <= 15);

                const float scale = g_afLookup[precision] * info.f_Range;

                for (int c = 0; c < info.i_Floats; ++c)
                {
                    const uint32_t v = bitPtr.ReadBits(precision + 1);
                    const bool isConst = (v & 1) != 0;
                    const uint32_t base = v >> 1;

                    if (isConst)
                    {
                        const float value = static_cast<float>(base) * scale + info.f_Low;
                        for (int m = 0; m < 9; ++m)
                            *pData++ = value;
                    }
                    else
                    {
                        const int bitCount = bitPtr.ReadBits(4) + 1;
                        const uint32_t firstDelta = firstStream.ReadBits(bitCount);
                        const uint32_t lastDelta = lastStream.ReadBits(bitCount);

                        int32_t values[9] = {};
                        values[0] = static_cast<int32_t>(base) + static_cast<int32_t>(firstDelta);
                        values[8] = static_cast<int32_t>(base) + static_cast<int32_t>(lastDelta);

                        const int midBitCount = midStream.ReadBits(4);
                        for (int m = 0; m < blockSamples - 1; ++m)
                        {
                            const uint32_t delta = midStream.ReadBits(midBitCount);
                            values[m + 1] = values[m] + SignExtend(delta, midBitCount);
                        }

                        const int valueCount = (blockSamples == 8) ? 9 : blockSamples;
                        for (int m = 0; m < valueCount; ++m)
                            *pData++ = static_cast<float>(values[m]) * scale + info.f_Low;

                        pData += 9 - valueCount;
                    }
                }
            }
        }

        ZASSERT((pData - pDataStart) <= 104 * 9);
    }
}
