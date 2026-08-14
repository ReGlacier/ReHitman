#include <Glacier/Render/Bitmap/ZBitmap32.h>
#include <Glacier/ZUniMemory.h>


namespace Glacier
{
    ZColorMask::ZColorMask() = default;

    uint32_t ZColorMask::SrcToDst(uint32_t dwColor, bool bAlphaInvert)
    {
        uint32_t dwAlpha = (dwColor & m_Src.dwMask[3]) >> m_Src.dwPos[3] << m_Src.dwSize[3];
        if (bAlphaInvert)
        {
            dwAlpha = 255 - dwAlpha;
        }

        return ((dwColor & m_Src.dwMask[0]) >> m_Src.dwPos[0] << m_Src.dwSize[0] >> m_Dst.dwSize[0] << m_Dst.dwPos[0])
             + ((dwColor & m_Src.dwMask[1]) >> m_Src.dwPos[1] << m_Src.dwSize[1] >> m_Dst.dwSize[1] << m_Dst.dwPos[1])
             + ((dwColor & m_Src.dwMask[2]) >> m_Src.dwPos[2] << m_Src.dwSize[2] >> m_Dst.dwSize[2] << m_Dst.dwPos[2])
             + (dwAlpha >> m_Dst.dwSize[3] << m_Dst.dwPos[3]);
    }

    void ZColorMask::InitShifters(ZColorMask::_ZColorMask& Mask)
    {
        for (int i = 0; i < 4; ++i)
        {
            uint32_t dwMask = Mask.dwMask[i];
            Mask.dwPos[i] = 0;
            Mask.dwSize[i] = 8;

            if (dwMask)
            {
                while (dwMask && !(dwMask & 1))
                {
                    dwMask >>= 1;
                    ++Mask.dwPos[i];
                }

                if (dwMask)
                {
                    int dwShift = 8;
                    do
                    {
                        dwMask >>= 1;
                        --dwShift;
                    }
                    while (dwMask);

                    Mask.dwSize[i] = dwShift;
                }
            }
        }
    }

    ZBitmap32::ZBitmap32()
    {
        m_dwType = 'RGBA'; // PC: 1380401729
    }

    ZBitmap32::ZBitmap32(uint32_t* pData, int x, int y, int rMask, int gMask, int bMask, int aMask, bool bInvertAlpha)
    {
        m_dwType = 'RGBA';
        m_dwSize[0] = x;
        m_dwSize[1] = y;
        SetMIPLevels(1);

        ZColorMask colorMask;
        colorMask.m_Src.dwMask[0] = rMask;
        colorMask.m_Src.dwMask[1] = gMask;
        colorMask.m_Src.dwMask[2] = bMask;
        colorMask.m_Src.dwMask[3] = aMask;
        colorMask.InitShifters(colorMask.m_Src);

        colorMask.m_Dst.dwMask[0] = 0xFF;
        colorMask.m_Dst.dwMask[1] = 0xFF00;
        colorMask.m_Dst.dwMask[2] = 0xFF0000;
        colorMask.m_Dst.dwMask[3] = 0xFF000000;
        colorMask.InitShifters(colorMask.m_Dst);

        const int lPixelCount = x * y;
        auto* pBuffer = static_cast<uint32_t*>(ZUniMemory::Allocate(4 * lPixelCount));
        for (int i = 0; i < lPixelCount; ++i)
        {
            pBuffer[i] = colorMask.SrcToDst(pData[i], bInvertAlpha);
        }

        SetData(pBuffer, 4 * lPixelCount, x, y, 0, true);
        ZUniMemory::Free(pBuffer);
    }

    ZBitmap32::~ZBitmap32() = default;

    void ZBitmap32::GetRGBA(uint8_t* pDst, int px, int py, int dx, int dy, int pitch, int dwLevel) const
    {
        const TMipLevel* pMipLevel = const_cast<ZBitmap32*>(this)->GetMIPLevel(dwLevel);
        const uint32_t* pSrc = static_cast<const uint32_t*>(pMipLevel->pData) + px + py * pMipLevel->dwSizeX;

        for (int y = 0; y < dy; ++y)
        {
            for (int x = 0; x < dx; ++x)
            {
                reinterpret_cast<uint32_t*>(pDst)[x] = pSrc[x];
            }
            pSrc += pMipLevel->dwSizeX;
            pDst += pitch;
        }
    }

    void ZBitmap32::GetData(ZBitmap::TDataBlock* pData, int dwMipLevel)
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
        const uint32_t* pSrc = static_cast<const uint32_t*>(pMipLevel->pData) + pData->dwPosX + pMipLevel->dwSizeX * pData->dwPosY;

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
            pSrc += pMipLevel->dwSizeX;
            pDst += pData->dwPich;
        }
    }

    void ZBitmap32::PackMipLevel(int level, int sx, int sy, const uint32_t* pRGBA)
    {
        SetData(const_cast<uint32_t*>(pRGBA), 4 * sx * sy, sx, sy, level, true);
    }
}