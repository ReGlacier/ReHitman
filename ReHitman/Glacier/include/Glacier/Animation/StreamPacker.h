#pragma once

#include <Glacier/ReGlacier.h>
#include <Glacier/Animation/EBoneID.h>
#include <cstdint>


namespace Glacier
{
    /**
     * @brief Static helpers for unpacking the engine's bit-packed animation streams (ANM).
     *
     * Animation curves are stored in a compact bitstream that is split into
     * 8-frame blocks. Each curve channel (angle, float, quaternion, or a
     * combined vector + quaternion) is delta encoded across a block so that only
     * the first value, the last value and the per-frame deltas have to be stored.
     *
     * @c StreamPacker::DePackBlock decodes a whole block into a reusable
     * @ref BlockCache, while @ref DePackBlockVector and @ref DePackVector decode
     * a single frame (interpolating between two neighbouring keyframes on the fly).
     */
    struct StreamPacker
    {
        /**
         * @brief The data types a packed channel can carry.
         *
         * These values are the low nibble of the per-channel "layout" byte and
         * select the decoding parameters from the static layout table.
         */
        enum DataType
        {
            eAngle = 0x0,        ///< An angle that wraps around 2*pi.
            eFloat = 0x1,        ///< A plain bounded float.
            eGenericFloat = 0x2, ///< A float whose low/range are read from the stream itself.
            eQuat = 0x3,         ///< A compressed quaternion.
            eBone = 0x4,         ///< First combined (vector + quaternion) layout.
        };

        /**
         * @brief Decoded storage for one 8-frame block of a single channel.
         *
         * @c DePackBlock fills this cache, which is then reused by
         * @ref DePackBlockVector to sample frames without re-decoding the block.
         */
        struct BlockCache
        {
            int16_t m_Block;   ///< Index of the decoded block.
            int16_t m_Size;    ///< Number of entries (bone ids) in this block.
            uint16_t* m_Ids;   ///< Bone id of each entry.
            uint8_t* m_Lps;    ///< Per-entry layout byte (layout nibble + precision nibble).
            float* m_Data;     ///< Decoded keyframes, laid out per entry.
        };
        RE_VERIFY_SIZE(BlockCache, 0x10);

        /**
         * @brief Runtime description of a single data channel.
         *
         * Pairs a @ref DataType with a quantization precision. Not used for
         * decoding itself; it is how the engine configures channels before packing.
         */
        struct DataInfo
        {
            /// Assigns the channel type and precision.
            void Set(DataType eType, float fPrec)
            {
                m_Type = eType;
                f_Prec = fPrec;
            }

            DataType m_Type; ///< Channel data type.
            float f_Prec;    ///< Quantization precision (bits).
        };

        /**
         * @brief Per-layout decode parameters used by @ref DePackVector.
         */
        struct DePackVector
        {
            /**
             * @brief Static decode descriptor for one layout index.
             *
             * Describes how a quantized value maps back to a real value:
             * @c value * (_range / (2^bits - 1)) + _low, plus how many vector and
             * quaternion components the layout contains.
             */
            struct DepackInfo3
            {
                float _low;    ///< Lower bound of the decoded value range.
                float _range;  ///< Size of the decoded value range.
                char floats;   ///< Number of vector (float) components.
                char quats;    ///< Number of quaternion components (0 or 1).
                RE_ADD_PADDING(2);
            };
        };

        /**
         * @brief A bit-level cursor over a byte buffer.
         *
         * @c bit counts individual bits from the start of @c base (little-endian
         * bit order: the least-significant bit of each byte comes first). Used to
         * walk the several interleaved bitstreams that make up a packed block.
         */
        struct BitPtr
        {
            uint8_t* base; ///< Backing byte buffer.
            uint32_t bit;  ///< Current bit offset from the start of @c base.

            /// Creates a cursor at bit 0 of @c ptr.
            BitPtr(uint8_t* ptr);

            /// Copies a cursor (both base and current bit offset).
            BitPtr(const BitPtr& copy);

            /// Returns a pointer to the byte that currently contains @c bit.
            uint8_t* GetBytePtr();

            /// Reads @c bitCount bits and advances @c bit past them.
            uint32_t ReadBits(int bitCount)
            {
                const uint32_t window = static_cast<uint32_t>(base[bit >> 3])
                                      | (static_cast<uint32_t>(base[(bit >> 3) + 1]) << 8)
                                      | (static_cast<uint32_t>(base[(bit >> 3) + 2]) << 16);
                const uint32_t value = (window >> (bit & 7)) & ((1u << bitCount) - 1);
                bit += bitCount;
                return value;
            }

            /// Advances @c bit without reading anything.
            void Skip(int bitCount)
            {
                bit += bitCount;
            }
        };

        /**
         * @brief Decodes a single interpolated pose directly from the packed stream.
         *
         * Reads the block header, seeks to the requested frame and decodes the two
         * neighbouring keyframes on the fly, then interpolates between them.
         *
         * @param data      Packed block stream.
         * @param samples   Total number of frames in the animation.
         * @param frame2    Requested frame (may be fractional).
         * @param pId2Pos   Bone-id to output-offset lookup (or @c nullptr entries).
         * @param pSamples  Output buffer receiving the decoded pose.
         * @param bailAt    Stop decoding once a bone id reaches this value.
         */
        static void DePackVector(uint8_t* data, int samples, float frame2, int16_t* pId2Pos, float* pSamples, int bailAt);

        /**
         * @brief Decodes a single interpolated pose from a previously decoded block.
         *
         * This is the cached variant of @ref DePackVector: instead of parsing the
         * bitstream it samples the keyframes already stored in @c pBlockCache.
         *
         * @param pBlockCache The block decoded by @ref DePackBlock for the frame's block.
         * @param samples     Total number of frames in the animation.
         * @param frame2      Requested frame (may be fractional).
         * @param pId2Pos     Bone-id to output-offset lookup.
         * @param pSamples    Output buffer receiving the decoded pose.
         * @param bailAt      Stop decoding once a bone id reaches this value.
         */
        static void DePackBlockVector(StreamPacker::BlockCache* pBlockCache, int samples, float frame2, int16_t* pId2Pos, float* pSamples, int bailAt);

        /**
         * @brief Reads a vector channel's leading bone-id count and advances the buffer.
         *
         * @param pBuffer The stream position; the first byte is the id count.
         * @param eBone   Receives the bone-id count read from @c pBuffer.
         * @return A pointer just past the count byte (start of the id list).
         */
        static uint8_t* GetVectorIds(uint8_t* pBuffer, EBoneID& eBone);

        /**
         * @brief Computes the block index that contains the given frame.
         *
         * @param lFrame Total frame count of the animation.
         * @param fFrame Requested frame.
         * @return The 8-frame block index (0 based).
         */
        static int32_t GetBlock(int32_t lFrame, float fFrame);

        /**
         * @brief Decodes one whole 8-frame block into a reusable cache.
         *
         * Parses the block header and the three auxiliary delta streams, then
         * reconstructs every keyframe of every channel into @c pBlockCache.
         *
         * @param data        Packed block stream.
         * @param samples     Total number of frames in the animation.
         * @param lBlock      Index of the block to decode.
         * @param pBlockCache Output cache that receives the decoded block.
         */
        static void DePackBlock(uint8_t* data, int samples, int lBlock, StreamPacker::BlockCache* pBlockCache);
    };
}
