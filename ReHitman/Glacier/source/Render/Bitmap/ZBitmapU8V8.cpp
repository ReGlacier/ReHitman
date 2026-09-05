#include <Glacier/Render/Bitmap/ZBitmapU8V8.h>
#include <Glacier/Render/Bitmap/ZBitmap32.h>
#include <Glacier/ZUniMemory.h>

#include <cstring>

namespace Glacier
{
    ZBitmapU8V8::ZBitmapU8V8() 
    {
        m_dwType = 'U8V8';
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

    ZBitmapU8V8::~ZBitmapU8V8() = default;

    void ZBitmapU8V8::GetRGBA(uint8_t* pDst, int px, int py, int dx, int dy, int pitch, int dwLevel) const
    {
        const TMipLevel* pMipLevel = const_cast<ZBitmapU8V8*>(this)->GetMIPLevel(dwLevel);
        const uint16_t* pSrc = static_cast<const uint16_t*>(pMipLevel->pData) + px + py * pMipLevel->dwSizeX;

        for (int y = 0; y < dy; ++y)
        {
            for (int x = 0; x < dx; ++x)
            {
                // Opaque texel: R=255, G=U, B=V, A=255 (PC: (v << 8) | 0xFF0000FF)
                reinterpret_cast<uint32_t*>(pDst)[x] = (static_cast<uint32_t>(pSrc[x]) << 8) | 0xFF0000FFu;
            }
            pSrc += pMipLevel->dwSizeX;
            pDst += pitch;
        }
    }

    void ZBitmapU8V8::GetData(ZBitmap::TDataBlock* pData, int dwMipLevel)
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
        const uint16_t* pSrc = static_cast<const uint16_t*>(pMipLevel->pData) + pData->dwPosX + pMipLevel->dwSizeX * pData->dwPosY;

        for (int y = 0; y < pData->dwSizeY; ++y)
        {
            for (int x = 0; x < pData->dwSizeX; ++x)
            {
                // The raw U8V8 texel is converted directly: U -> R channel, V -> G channel.
                if (pData->dwBitCount == 32)
                {
                    reinterpret_cast<uint32_t*>(pDst)[x] = colorMask.SrcToDst(pSrc[x], pData->bInvertAlpha);
                }
                else
                {
                    reinterpret_cast<uint16_t*>(pDst)[x] = static_cast<uint16_t>(colorMask.SrcToDst(pSrc[x], pData->bInvertAlpha));
                }
            }
            pSrc += pMipLevel->dwSizeX;
            pDst += pData->dwPich;
        }
    }

    void ZBitmapU8V8::Swizzle()
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

            auto* pSwizzled = static_cast<uint16_t*>(ZUniMemory::Allocate(2 * pMipLevel->dwSizeX * pMipLevel->dwSizeY));
            const auto* pSrc = static_cast<const uint16_t*>(pMipLevel->pData);

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

            memcpy(pMipLevel->pData, pSwizzled, 2 * pMipLevel->dwSizeX * pMipLevel->dwSizeY);
            ZUniMemory::Free(pSwizzled);
        }
    }

    void ZBitmapU8V8::PackMipLevel(int level, int sx, int sy, const uint32_t* pRGBA)
    {
        const int lSize = 2 * sx * sy;
        auto* pBuffer = static_cast<uint16_t*>(ZUniMemory::Allocate(lSize));

        // U is taken from the G channel, V from the A channel of the source RGBA pixels.
        const uint8_t* pSrc = reinterpret_cast<const uint8_t*>(pRGBA) + 1;
        uint16_t* pDst = pBuffer;
        for (int y = 0; y < sy; ++y)
        {
            for (int x = 0; x < sx; ++x)
            {
                *pDst++ = static_cast<uint16_t>(pSrc[0] | (static_cast<uint16_t>(pSrc[2]) << 8));
                pSrc += 4;
            }
        }

        SetData(pBuffer, lSize, sx, sy, level, true);
        ZUniMemory::Free(pBuffer);
    }
}
