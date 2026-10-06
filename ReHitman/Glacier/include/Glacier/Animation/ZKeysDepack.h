#pragma once

#include <Glacier/ReGlacier.h>
#include <cstdint>


namespace Glacier
{
    // Decompression / compression of the key-frame data blocks used by ZRunMatPosAnim.
    //
    // A key block is laid out as:
    //   [0] int32 lNrFrames
    //   [1] int32 lFrameRateDivisor  (tick-per-frame deltas of the time table)
    //   [2] int32 lNrPosKeys         (position key count)
    //   [3] int32 lNrQuatKeys        (quaternion key count)
    //   then three tables:
    //   - offset table (bit-packed, pOffsets[0] bits per entry; maps a frame -> key index)
    //   - time table   (same bit width; maps a key index -> absolute tick time)
    //   - key data     (interleaved pos Vec3 floats / quat int16x4)
    class ZKeysDepack
    {
    public:
        // Depacking (PC)
        void DepackQ4(float* q0, float fTime, unsigned int lNrFrames, const char* pOffsets, const char* pTimes, const int16_t* pKeys);
        void DepackV3(float* p0, float fTime, unsigned int lNrFrames, const char* pOffsets, const char* pTimes, const float* pKeys);
        int32_t ReadOffset(const char* pOffsets, int lOffsetNr);
        void DepackQuatKey(float* pQuat, const int16_t* pKeys);

        // Packing (test / inverse of Depack)
        static void PackQuatKey(int16_t* pKeys, const float* pQuat);
        static void PackV3Keys(float* pKeys, int lNrKeys, const float* pValues);
        static void PackQ4Keys(int16_t* pKeys, int lNrKeys, const float* pValues);
        static void WriteOffset(char* pOffsets, int lBitsPerOffset, int lOffsetNr, int lValue);
        static uint32_t OffsetTableBufferBytes(int lBitsPerOffset, int lCount);
        static void PackKeyTables(
            char* pOffsets,       // [out] bit-packed offset table
            char* pTimes,         // [out] bit-packed time table
            int lBitsPerOffset,
            int lBitsPerTime,
            int lNrFrames,        // number of frames
            int lNrKeys,          // number of keys
            const int* pKeyFrames);// keyFrames[i] = frame index of key i
    };
}
