#include <Glacier/Render/Bitmap/ZBitmapPalOpac.h>
#include <Glacier/ZUniMemory.h>

#include <cstring>


namespace Glacier
{
    ZBitmapPalOpac::ZBitmapPalOpac()
    {
        m_dwType = 'PALO';
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
        m_pOpac = nullptr;
        m_bOpacAllocated = false;
    }

    ZBitmapPalOpac::~ZBitmapPalOpac()
    {
        if (m_bOpacAllocated && m_pOpac)
        {
            ZUniMemory::Free(m_pOpac);
            m_bOpacAllocated = false;
            m_pOpac = nullptr;
        }

        if (m_bPalAllocated && m_pPal)
        {
            ZUniMemory::Free(m_pPal);
            m_bPalAllocated = false;
            m_pPal = nullptr;
        }
    }

    void ZBitmapPalOpac::GetRGBA(uint8_t* pDst, int px, int py, int dx, int dy, int pitch, int dwLevel) const
    {
        const TMipLevel* pMipLevel = const_cast<ZBitmapPalOpac*>(this)->GetMIPLevel(dwLevel);
        const uint8_t* pSrc = static_cast<const uint8_t*>(pMipLevel->pData) + px + py * pMipLevel->dwSizeX;
        const uint8_t* pOpac = reinterpret_cast<const uint8_t*>(m_pOpac) + px + py * pMipLevel->dwSizeX;

        for (int y = 0; y < dy; ++y)
        {
            for (int x = 0; x < dx; ++x)
            {
                // Alpha comes from the opacity buffer, palette alpha is masked off.
                reinterpret_cast<uint32_t*>(pDst)[x] =
                    (m_pPal[pSrc[x]] & 0xFFFFFF) | (static_cast<uint32_t>(pOpac[x]) << 24);
            }
            pSrc += pMipLevel->dwSizeX;
            pOpac += pMipLevel->dwSizeX;
            pDst += pitch;
        }
    }
    
    uint32_t ZBitmapPalOpac::GetBinSize()
    {
        return ZBitmapPal::GetBinSize() + static_cast<uint32_t>(GetSizeX() * GetSizeY());
    }

    void ZBitmapPalOpac::SaveBin(char* pBuffer)
    {
        ZBitmap::SaveBin(pBuffer);

        char* pDst = pBuffer + ZBitmap::GetBinSize();
        *reinterpret_cast<int32_t*>(pDst) = m_lPalSize;
        pDst += sizeof(int32_t);

        memcpy(pDst, m_pPal, sizeof(uint32_t) * m_lPalSize);
        pDst += sizeof(uint32_t) * m_lPalSize;

        memcpy(pDst, m_pOpac, static_cast<size_t>(GetSizeX() * GetSizeY()));
    }

    void ZBitmapPalOpac::LoadBin(const char* pBuffer)
    {
        ZBitmap::LoadBin(pBuffer);

        const char* pSrc = pBuffer + ZBitmap::GetBinSize();
        m_lPalSize = *reinterpret_cast<const int32_t*>(pSrc);
        pSrc += sizeof(int32_t);

        m_pPal = reinterpret_cast<uint32_t*>(const_cast<char*>(pSrc));
        m_pOpac = reinterpret_cast<uint32_t*>(const_cast<char*>(pSrc + sizeof(uint32_t) * m_lPalSize));
    }
}
