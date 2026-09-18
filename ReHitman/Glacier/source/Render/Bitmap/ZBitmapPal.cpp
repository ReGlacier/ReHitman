#include <Glacier/Render/Bitmap/ZBitmapPal.h>
#include <Glacier/Render/Bitmap/ZBitmap32.h>
#include <Glacier/ZUniMemory.h>

#include <cstring>

#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    ZBitmapPal::ZBitmapPal() 
        : ZBitmap()
    {
        m_dwType = 'PALN';
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
        m_pPal = nullptr;
        m_bPalAllocated = false;
    }

    ZBitmapPal::~ZBitmapPal()
    {
        if (m_bPalAllocated && m_pPal)
        {
            ZUniMemory::Free(m_pPal);
            m_pPal = nullptr;
        }
    }

    void ZBitmapPal::Create(const ZBitmap& Bitmap, bool bBitmap)
    {
        ZASSERT(false); // PC: Create is not supported for paletted bitmaps
    }

    void ZBitmapPal::GetRGBA(uint8_t* pDst, int px, int py, int dx, int dy, int pitch, int dwLevel) const
    {
        const TMipLevel* pMipLevel = const_cast<ZBitmapPal*>(this)->GetMIPLevel(dwLevel);
        ZASSERT(pMipLevel->dwSizeX == dx);
        ZASSERT(pMipLevel->dwSizeY == dy);

        const uint8_t* pSrc = static_cast<const uint8_t*>(pMipLevel->pData) + px + py * pMipLevel->dwSizeX;

        for (int y = 0; y < dy; ++y)
        {
            for (int x = 0; x < dx; ++x)
            {
                reinterpret_cast<uint32_t*>(pDst)[x] = m_pPal[pSrc[x]];
            }
            pSrc += pMipLevel->dwSizeX;
            pDst += pitch;
        }
    }

    void ZBitmapPal::GetData(ZBitmap::TDataBlock* pData, int dwMipLevel)
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
                const uint32_t dwPixel = m_pPal[pSrc[x]];
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

    uint32_t ZBitmapPal::GetBinSize()
    {
        return ZBitmap::GetBinSize() + static_cast<uint32_t>(sizeof(uint32_t)) * (m_lPalSize + 1);
    }

    void ZBitmapPal::SaveBin(char* pBuffer)
    {
        ZBitmap::SaveBin(pBuffer);

        char* pDst = pBuffer + ZBitmap::GetBinSize();
        *reinterpret_cast<int32_t*>(pDst) = m_lPalSize;
        memcpy(pDst + sizeof(int32_t), m_pPal, sizeof(uint32_t) * m_lPalSize);
    }

    void ZBitmapPal::LoadBin(const char* pBuffer)
    {
        ZBitmap::LoadBin(pBuffer);

        const char* pSrc = pBuffer + ZBitmap::GetBinSize();
        m_lPalSize = *reinterpret_cast<const int32_t*>(pSrc);
        m_pPal = reinterpret_cast<uint32_t*>(const_cast<char*>(pSrc + sizeof(int32_t)));
    }
    
    void ZBitmapPal::Swizzle()
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
    
    void ZBitmapPal::PackMipLevel(int level, int sx, int sy, const uint32_t* pRGBA)
    {
        // Do nothing
    }

    int32_t ZBitmapPal::GetPalSize() const
    {
        return m_lPalSize;
    }

    uint32_t* ZBitmapPal::GetPal() const
    {
        return m_pPal;
    }
    
    void ZBitmapPal::SetPalette(uint32_t* pPalette, int32_t lPaletteSize, bool bNeedCopyPalette)
    {
        if (m_bPalAllocated)
        {
            ZUniMemory::Free(m_pPal);
            m_pPal = nullptr;
            m_bPalAllocated = false;
        }

        m_lPalSize = lPaletteSize;

        if (bNeedCopyPalette)
        {
            uint32_t* pPalettePtr = (uint32_t*)ZUniMemory::Allocate(sizeof(uint32_t) * lPaletteSize);
            std::memcpy(pPalettePtr, pPalette, sizeof(uint32_t) * m_lPalSize);

            m_pPal = pPalettePtr;
            m_bPalAllocated = true;
        }
        else
        {
            m_pPal = pPalette;
        }
    }
}
