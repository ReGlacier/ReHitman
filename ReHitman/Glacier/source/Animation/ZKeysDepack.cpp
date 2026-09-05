#include <Glacier/Animation/ZKeysDepack.h>
#include <Glacier/ZSTL/ZMath.h>
#include <Glacier/ReGlacier.h>

#include <cmath>


namespace Glacier
{
    // PC 0x439880. Reads a variable-width bitmap (pOffsets[0] bits per offset) at lOffsetNr-1.
    int32_t ZKeysDepack::ReadOffset(const char* pOffsets, int lOffsetNr)
    {
        if (!lOffsetNr)
            return 0;

        const int lBits = static_cast<unsigned char>(pOffsets[0]);
        const int lBitPos = lBits * (lOffsetNr - 1);
        const int lBitInByte = lBitPos & 7;
        const auto* pData = reinterpret_cast<const uint8_t*>(pOffsets) + 1 + (lBitPos >> 3);

        // PC 0x439880 loads a 24-bit big-endian window and extracts the requested MSB-first
        // field. The two lookahead bytes make fields crossing byte boundaries safe.
        const uint32_t lWindow = (static_cast<uint32_t>(pData[0]) << 16)
                               | (static_cast<uint32_t>(pData[1]) << 8)
                               | static_cast<uint32_t>(pData[2]);
        return static_cast<int32_t>((lWindow >> (24 - lBitInByte - lBits)) & ((1u << lBits) - 1u));
    }

    // PC 0x439940. Decompresses a 3-float vector key at fTime, interpolated to the next key.
    void ZKeysDepack::DepackV3(float* p0, float fTime, unsigned int lNrFrames, const char* pOffsets, const char* pTimes, const float* pKeys)
    {
        const int lFrame = static_cast<int>(fTime);
        const int lOffset = ReadOffset(pOffsets, lFrame);
        const int lTime0 = ReadOffset(pTimes, lOffset);

        const float* pKey0 = &pKeys[3 * lOffset];
        p0[0] = pKey0[0];
        p0[1] = pKey0[1];
        p0[2] = pKey0[2];

        if (lFrame < static_cast<int>(lNrFrames - 1))
        {
            const int lTime1 = ReadOffset(pTimes, lOffset + 1);
            if (lTime1 <= lFrame)
                ZASSERT(false); // PC: __debugbreak()

            const float fLerp = (fTime - static_cast<float>(lTime0)) / static_cast<float>(lTime1 - lTime0);
            if (fLerp != 0.0f)
            {
                const float* pKey1 = &pKeys[3 * (lOffset + 1)];
                p0[0] = (pKey1[0] - p0[0]) * fLerp + p0[0];
                p0[1] = (pKey1[1] - p0[1]) * fLerp + p0[1];
                p0[2] = (pKey1[2] - p0[2]) * fLerp + p0[2];
            }
        }
    }

    // PC 0x439A20. Decompresses a quaternion key at fTime, splerped towards the next key.
    void ZKeysDepack::DepackQ4(float* q0, float fTime, unsigned int lNrFrames, const char* pOffsets, const char* pTimes, const int16_t* pKeys)
    {
        const int lFrame = static_cast<int>(fTime);
        const int lOffset = ReadOffset(pOffsets, lFrame);
        const int lTime0 = ReadOffset(pTimes, lOffset);

        const int16_t* pKey0 = &pKeys[4 * lOffset];
        DepackQuatKey(q0, pKey0);

        if (lFrame < static_cast<int>(lNrFrames - 1))
        {
            const int lTime1 = ReadOffset(pTimes, lOffset + 1);
            float q1[4];
            DepackQuatKey(q1, pKey0 + 4);
            if (lTime1 <= lFrame)
                ZASSERT(false); // PC: __debugbreak()

            const float fLerp = (fTime - static_cast<float>(lTime0)) / static_cast<float>(lTime1 - lTime0);
            if (fLerp != 0.0f)
                qpul(q0, q0, q1, fLerp);
        }
    }

    // PC 0x4398E0. Scales the 16-bit quaternion key: real (w) = key[0], ijk = key[1..3], by 1/32506.
    void ZKeysDepack::DepackQuatKey(float* pQuat, const int16_t* pKeys)
    {
        constexpr float kInvKeyScale = 0.000030757874f; // 1/32506.27
        pQuat[3] = static_cast<float>(pKeys[0]) * kInvKeyScale; // real (w)
        pQuat[0] = static_cast<float>(pKeys[1]) * kInvKeyScale; // ijk.X (i)
        pQuat[1] = static_cast<float>(pKeys[2]) * kInvKeyScale; // ijk.Y (j)
        pQuat[2] = static_cast<float>(pKeys[3]) * kInvKeyScale; // ijk.Z (k)
    }

    // Inverse of DepackQuatKey: quantizes a normalized quat {i,j,k,w} into four int16 keys.
    void ZKeysDepack::PackQuatKey(int16_t* pKeys, const float* pQuat)
    {
        constexpr float kKeyScale = 32512.4f; // 1 / 0.000030757874
        auto clamp = [](float v) -> float {
            if (v < -1.0f) return -1.0f;
            if (v > 1.0f) return 1.0f;
            return v;
        };
        pKeys[0] = static_cast<int16_t>(clamp(pQuat[3]) * kKeyScale); // real (w)
        pKeys[1] = static_cast<int16_t>(clamp(pQuat[0]) * kKeyScale); // ijk.X (i)
        pKeys[2] = static_cast<int16_t>(clamp(pQuat[1]) * kKeyScale); // ijk.Y (j)
        pKeys[3] = static_cast<int16_t>(clamp(pQuat[2]) * kKeyScale); // ijk.Z (k)
    }

    void ZKeysDepack::PackV3Keys(float* pKeys, int lNrKeys, const float* pValues)
    {
        for (int i = 0; i < lNrKeys * 3; ++i)
            pKeys[i] = pValues[i];
    }

    void ZKeysDepack::PackQ4Keys(int16_t* pKeys, int lNrKeys, const float* pValues)
    {
        for (int i = 0; i < lNrKeys; ++i)
            PackQuatKey(pKeys + i * 4, pValues + i * 4);
    }

    // Bit-writes a value into the variable-width bitmap at entry lOffsetNr-1 (0 = not written).
    // The buffer carries the bit-width in byte 0 and the packed stream in bytes [1..]; entry i is
    // stored at bit position 8 + lBits*(i-1), matching the layout ReadOffset expects.
    void ZKeysDepack::WriteOffset(char* pOffsets, int lBitsPerOffset, int lOffsetNr, int lValue)
    {
        // Byte 0 stores the bit-width. The packed MSB-first stream starts at byte +1.
        pOffsets[0] = static_cast<char>(lBitsPerOffset);

        if (lOffsetNr <= 0)
            return;

        const int lBitPos = lBitsPerOffset * (lOffsetNr - 1);
        auto* pBytes = reinterpret_cast<uint8_t*>(pOffsets);

        // ReadOffset treats bytes [1..] as an MSB-first bit stream.
        for (int b = 0; b < lBitsPerOffset; ++b)
        {
            const int bPos = lBitPos + b;
            const uint8_t lMask = static_cast<uint8_t>(1u << (7 - (bPos & 7)));
            const int lSourceBit = lBitsPerOffset - 1 - b;
            if ((lValue >> lSourceBit) & 1)
                pBytes[1 + (bPos >> 3)] |= lMask;
            else
                pBytes[1 + (bPos >> 3)] &= static_cast<uint8_t>(~lMask);
        }
    }

    uint32_t ZKeysDepack::OffsetTableBufferBytes(int lBitsPerOffset, int lCount)
    {
        const uint32_t lTotalBits = static_cast<uint32_t>(lBitsPerOffset * lCount);
        // Width byte + payload + two zero lookahead bytes required by ReadOffset's 24-bit load.
        return 1 + (lTotalBits + 7) / 8 + 2;
    }

    // Packs the frame->key offset table and the key->time table for the given key list.
    void ZKeysDepack::PackKeyTables(
        char* pOffsets, char* pTimes,
        int lBitsPerOffset, int lBitsPerTime, int lNrFrames, int lNrKeys,
        const int* pKeyFrames)
    {
        // Entry 0 is implicit and ReadOffset(..., 0) always returns zero.
        for (int f = 1; f < lNrFrames; ++f)
        {
            int lKeyIdx = 0;
            for (int k = 0; k < lNrKeys; ++k)
            {
                if (pKeyFrames[k] <= f)
                    lKeyIdx = k;
            }
            WriteOffset(pOffsets, lBitsPerOffset, f, lKeyIdx);
        }

        // DepackV3/Q4 compare these values directly with floor(fTime), so this table contains
        // frame indices, not TIMETYPE ticks.
        for (int i = 1; i < lNrKeys; ++i)
            WriteOffset(pTimes, lBitsPerTime, i, pKeyFrames[i]);
    }
}
