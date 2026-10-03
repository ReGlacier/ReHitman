#pragma once

#include <cstdint>


namespace Glacier
{
    // Shared DXT block codecs (PC zbitmap.cpp).
    // Decoders write 32-bit pixels in B|G<<8|R<<16|A<<24 order, pitch is in pixels.
    void Decode4x4DXT_RGB(uint32_t* pDst, const uint16_t* pBlock, int pitch);
    void Decode4x4DXT3(uint32_t* pDst, const uint16_t* pBlock, int pitch);

    // Range-fit block compressors. The 4x4 block is read from pSrc at (x, y) with
    // the given pitch (in pixels). pScratch receives the reconstructed 4x4 block,
    // pOut receives 8 bytes (DXT1) / 16 bytes (DXT3) of compressed data.
    void CompressBlockDXT1(int bAlpha, int pitch, int x, int y, const uint32_t* pSrc, uint32_t* pScratch, uint32_t* pOut);
    void CompressBlockDXT3(int bAlpha, int pitch, int x, int y, const uint32_t* pSrc, uint32_t* pScratch, uint32_t* pOut);
}
