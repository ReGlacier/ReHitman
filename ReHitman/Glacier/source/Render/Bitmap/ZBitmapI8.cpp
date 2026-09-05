#include <Glacier/Render/Bitmap/ZBitmapI8.h>
#include <Glacier/Render/Bitmap/ZBitmap32.h>
#include <Glacier/ZUniMemory.h>

#include <cstring>

namespace Glacier
{
    ZBitmapI8::ZBitmapI8()
    {
        m_dwType = 'I8  '; // PC: 1228415008
    }

    ZBitmapI8::~ZBitmapI8() = default;

    void ZBitmapI8::GetRGBA(uint8_t* pDst, int px, int py, int dx, int dy, int pitch, int dwLevel) const
    {
        const TMipLevel* pMipLevel = const_cast<ZBitmapI8*>(this)->GetMIPLevel(dwLevel);
        const uint8_t* pSrc = static_cast<const uint8_t*>(pMipLevel->pData) + px + py * pMipLevel->dwSizeX;

        for (int y = 0; y < dy; ++y)
        {
            for (int x = 0; x < dx; ++x)
            {
                reinterpret_cast<uint32_t*>(pDst)[x] = 0x01010101u * pSrc[x];
            }
            pSrc += pMipLevel->dwSizeX;
            pDst += pitch;
        }
    }

    void ZBitmapI8::GetData(ZBitmap::TDataBlock* pData, int dwMipLevel)
    {
        const TMipLevel* pMipLevel = GetMIPLevel(dwMipLevel);

        ZColorMask colorMask;
        colorMask.m_Src.dwMask[0] = 0xFF;
        colorMask.m_Src.dwMask[1] = 0xFF00;
        colorMask.m_Src.dwMask[2] = 0xFF0000;
        colorMask.m_Src.dwMask[3] = 0xFF000000;
        colorMask.InitShifters(colorMask.m_Src);

        colorMask.m_Dst.dwMask[0] = pData->dwMaskR;
        colorMask.m_Dst.dwMask[1] = pData->dwMaskG;
        colorMask.m_Dst.dwMask[2] = pData->dwMaskB;
        colorMask.m_Dst.dwMask[3] = pData->dwMaskA;
        colorMask.InitShifters(colorMask.m_Dst);

        uint8_t* pDst = static_cast<uint8_t*>(pData->pData);
        const uint8_t* pSrc = static_cast<const uint8_t*>(pMipLevel->pData) + pData->dwPosX + pMipLevel->dwSizeX * pData->dwPosY;

        for (int y = 0; y < pData->dwSizeY; ++y)
        {
            for (int x = 0; x < pData->dwSizeX; ++x)
            {
                // Opaque grayscale: R=G=B=v, A=255 (PC: 65793 * v - 0x1000000)
                const uint32_t dwPixel = 0x010101u * pSrc[x] - 0x01000000u;
                if (pData->dwBitCount == 16)
                {
                    reinterpret_cast<uint16_t*>(pDst)[x] = static_cast<uint16_t>(colorMask.SrcToDst(dwPixel, pData->bInvertAlpha));
                }
                else
                {
                    reinterpret_cast<uint32_t*>(pDst)[x] = colorMask.SrcToDst(dwPixel, pData->bInvertAlpha);
                }
            }
            pSrc += pMipLevel->dwSizeX;
            pDst += pData->dwPich;
        }
    }

    void ZBitmapI8::Swizzle()
    {
        for (int i = 0; i < m_dwMipLevels; ++i)
        {
            TMipLevel* pMipLevel = &m_pMipLevels[i];

            // Morton swizzle masks (PC sub_43F7B0, third dimension is always 0)
            uint32_t dwMaskX = 0;
            uint32_t dwMaskY = 0;
            uint32_t dwTest = 1;
            uint32_t dwBit = 1;
            bool bGrowing = true;
            do
            {
                bGrowing = false;
                if (dwTest < static_cast<uint32_t>(pMipLevel->dwSizeX))
                {
                    dwMaskX |= dwBit;
                    dwBit <<= 1;
                    bGrowing = true;
                }
                if (dwTest < static_cast<uint32_t>(pMipLevel->dwSizeY))
                {
                    dwMaskY |= dwBit;
                    dwBit <<= 1;
                    bGrowing = true;
                }
                dwTest <<= 1;
            }
            while (bGrowing);

            auto* pSwizzled = static_cast<uint8_t*>(ZUniMemory::Allocate(pMipLevel->dwSizeX * pMipLevel->dwSizeY));
            const auto* pSrc = static_cast<const uint8_t*>(pMipLevel->pData);

            uint32_t dwSrcIndex = 0;
            uint32_t dwMortonY = 0;
            for (int y = 0; y < pMipLevel->dwSizeY; ++y)
            {
                uint32_t dwMortonX = 0;
                for (int x = 0; x < pMipLevel->dwSizeX; ++x)
                {
                    pSwizzled[dwMortonY | dwMortonX] = pSrc[dwSrcIndex++];
                    dwMortonX = (dwMortonX - dwMaskX) & dwMaskX;
                }
                dwMortonY = (dwMortonY - dwMaskY) & dwMaskY;
            }

            memcpy(pMipLevel->pData, pSwizzled, pMipLevel->dwSizeX * pMipLevel->dwSizeY);
            ZUniMemory::Free(pSwizzled);
        }
    }

    void ZBitmapI8::PackMipLevel(int level, int sx, int sy, const uint32_t* pRGBA)
    {
        const int lPixelCount = sx * sy;
        auto* pBuffer = static_cast<uint8_t*>(ZUniMemory::Allocate(lPixelCount));

        // Intensity is taken from the B channel of the source RGBA pixels.
        const uint8_t* pSrc = reinterpret_cast<const uint8_t*>(pRGBA) + 2;
        uint8_t* pDst = pBuffer;
        for (int y = 0; y < sy; ++y)
        {
            for (int x = 0; x < sx; ++x)
            {
                *pDst++ = *pSrc;
                pSrc += 4;
            }
        }

        SetData(pBuffer, lPixelCount, sx, sy, level, true);
        ZUniMemory::Free(pBuffer);
    }
}
