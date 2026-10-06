#include <Glacier/Render/Bitmap/ZBitmapDXTCommon.h>
#include <Glacier/Render/Bitmap/ZBitmapDXT3.h>
#include <Glacier/Render/Bitmap/ZBitmap32.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZBitmapDXT3::ZBitmapDXT3()
    {
        m_dwType = 'DXT3';
        m_dwSize[0] = 0;
        m_dwSize[1] = 0;
        m_dwMipLevels = 0;
        m_pMipLevels = nullptr;
        m_pszName = nullptr;
        m_dwId = 0;
        m_dwParams = 0;
        m_dwChecksum = 0;
        m_bNameAllocated = false;
        m_fScaleFactor = 1.0f;
    }

    ZBitmapDXT3::~ZBitmapDXT3() = default;

    void ZBitmapDXT3::GetData(ZBitmap::TDataBlock* pData, int dwMipLevel)
    {
        const TMipLevel* pMipLevel = GetMIPLevel(dwMipLevel);

        const int lSizeX = (pMipLevel->dwSizeX + 3) & ~3;
        const int lSizeY = (pMipLevel->dwSizeY + 3) & ~3;
        auto* pDecoded = static_cast<uint32_t*>(ZUniMemory::Allocate(4 * lSizeX * lSizeY));
        DecodeMipLevel(reinterpret_cast<int*>(pDecoded), dwMipLevel);

        ZColorMask colorMask;
        // Decoded pixels are in B|G<<8|R<<16|A<<24 order.
        colorMask.m_Src.dwMask[0] = 0xFF0000;
        colorMask.m_Src.dwMask[1] = 0xFF00;
        colorMask.m_Src.dwMask[2] = 0xFF;
        colorMask.m_Src.dwMask[3] = 0xFF000000;
        colorMask.InitShifters(colorMask.m_Src);

        colorMask.m_Dst.dwMask[0] = pData->dwMaskR;
        colorMask.m_Dst.dwMask[1] = pData->dwMaskG;
        colorMask.m_Dst.dwMask[2] = pData->dwMaskB;
        colorMask.m_Dst.dwMask[3] = pData->dwMaskA;
        colorMask.InitShifters(colorMask.m_Dst);

        uint8_t* pDst = static_cast<uint8_t*>(pData->pData);
        const uint32_t* pSrc = pDecoded + pData->dwPosX + lSizeX * pData->dwPosY;

        for (int y = 0; y < pData->dwSizeY; ++y)
        {
            for (int x = 0; x < pData->dwSizeX; ++x)
            {
                if (pData->dwBitCount == 16)
                {
                    reinterpret_cast<uint16_t*>(pDst)[x] = static_cast<uint16_t>(colorMask.SrcToDst(pSrc[x], pData->bInvertAlpha));
                }
                else
                {
                    reinterpret_cast<uint32_t*>(pDst)[x] = colorMask.SrcToDst(pSrc[x], pData->bInvertAlpha);
                }
            }
            pSrc += lSizeX;
            pDst += pData->dwPich;
        }

        ZUniMemory::Free(pDecoded);
    }

    void ZBitmapDXT3::DecodeMipLevel(int* pDst, int dwLevel) const
    {
        const TMipLevel* pMipLevel = const_cast<ZBitmapDXT3*>(this)->GetMIPLevel(dwLevel);

        const int lSizeX = (pMipLevel->dwSizeX + 3) & ~3;
        const int lSizeY = (pMipLevel->dwSizeY + 3) & ~3;
        if (lSizeY - 3 <= 0)
        {
            return;
        }

        const int lBlockRows = ((lSizeY - 4) >> 2) + 1;
        const auto* pBlock = static_cast<const uint8_t*>(pMipLevel->pData);
        uint32_t* pOut = reinterpret_cast<uint32_t*>(pDst);

        for (int by = 0; by < lBlockRows; ++by)
        {
            if (lSizeX - 3 > 0)
            {
                const int lBlockCols = ((lSizeX - 4) >> 2) + 1;
                const uint8_t* pSrc = pBlock;
                uint32_t* pOutBlock = pOut;
                for (int bx = 0; bx < lBlockCols; ++bx)
                {
                    Decode4x4DXT3(pOutBlock, reinterpret_cast<const uint16_t*>(pSrc), lSizeX);
                    pSrc += 16;
                    pOutBlock += 4;
                }
                pBlock += 16 * lBlockCols;
            }
            pOut += 4 * lSizeX;
        }
    }

    void ZBitmapDXT3::PackMipLevel(int level, int sx, int sy, const uint32_t* pRGBA)
    {
        const int lSizeX = (sx + 3) & ~3;
        const int lSizeY = (sy + 3) & ~3;
        const int lSize = (lSizeX * lSizeY) >> 2; // 16 bytes per 4x4 block
        auto* pBuffer = static_cast<uint32_t*>(ZUniMemory::Allocate(4 * lSize));

        if (lSizeY - 3 > 0)
        {
            int y0 = 0;
            int lRemY = sy;
            int lOutIndex = 0;
            do
            {
                if (lSizeX - 3 > 0)
                {
                    uint32_t* pOut = pBuffer + lOutIndex;
                    int x0 = 0;
                    int lRemX = sx;
                    const int lBlockCols = ((lSizeX - 4) >> 2) + 1;
                    do
                    {
                        uint32_t block[16];
                        uint32_t scratch[16];
                        uint32_t out[4];
                        for (int row = 0; row < 4; ++row)
                        {
                            // Edge blocks clamp to the last row/column of the source.
                            const int gy = row >= lRemY ? sy - 1 : y0 + row;
                            for (int col = 0; col < 4; ++col)
                            {
                                const int gx = col >= lRemX ? sx - 1 : x0 + col;
                                block[4 * row + col] = pRGBA[gx + sx * gy];
                            }
                        }
                        CompressBlockDXT3(0, 4, 0, 0, block, scratch, out);
                        pOut[0] = out[0];
                        pOut[1] = out[1];
                        pOut[2] = out[2];
                        pOut[3] = out[3];
                        pOut += 4;
                        x0 += 4;
                        lRemX -= 4;
                    }
                    while (x0 < lSizeX - 3);
                    lOutIndex += 4 * lBlockCols;
                }
                y0 += 4;
                lRemY -= 4;
            }
            while (y0 < lSizeY - 3);
        }

        SetData(pBuffer, 4 * lSize, sx, sy, level, true);
        ZUniMemory::Free(pBuffer);
    }
}
