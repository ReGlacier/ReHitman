#include <Glacier/Render/Bitmap/ZBitmap.h>
#include <Glacier/ZUniMemory.h>

#include <cstring>

#include <Glacier/ZUniAssert.h>


namespace Glacier
{
    static constexpr const char* s_EmptyString = "";

    ZBitmap::~ZBitmap()
    {
        for (int i = 0; i < m_dwMipLevels; ++i)
        {
            if (m_pMipLevels[i].bAllocated)
            {
                ZUniMemory::Free(m_pMipLevels[i].pData);
            }
        }

        if (m_pMipLevels)
        {
            ZUniMemory::Free(m_pMipLevels);
        }

        if (m_bNameAllocated)
        {
            ZUniMemory::Free(m_pszName);
        }
    }

    const char* ZBitmap::GetName()
    {
        return m_pszName;
    }

    int ZBitmap::GetType()
    {
        return m_dwType;
    }

    void ZBitmap::Create(const ZBitmap& Bitmap, bool bMipMap)
    {
        SetSizeX(Bitmap.GetSizeX());
        SetSizeY(Bitmap.GetSizeY());

        int lLevels = 1;
        if (bMipMap)
        {
            int lSizeX = GetSizeX() >> 1;
            int lSizeY = GetSizeY() >> 1;
            if (lSizeX)
            {
                do
                {
                    if (!lSizeY)
                    {
                        break;
                    }
                    lSizeX >>= 1;
                    ++lLevels;
                    lSizeY >>= 1;
                }
                while (lSizeX);
            }
        }
        SetMIPLevels(lLevels);

        if (lLevels > 1 && Bitmap.GetMipLevelCount() > 1 && Bitmap.GetMipLevelCount() == lLevels)
        {
            int lSizeX = GetSizeX();
            int lSizeY = GetSizeY();
            auto* pBuffer = static_cast<uint32_t*>(ZUniMemory::Allocate(4 * lSizeX * lSizeY));
            for (int i = 0; i < lLevels; ++i)
            {
                Bitmap.GetRGBA(reinterpret_cast<uint8_t*>(pBuffer), 0, 0, lSizeX, lSizeY, 4 * lSizeX, i);
                PackMipLevel(i, lSizeX, lSizeY, pBuffer);
                lSizeX = (lSizeX + 1) >> 1;
                lSizeY = (lSizeY + 1) >> 1;
            }
            ZUniMemory::Free(pBuffer);
        }
        else
        {
            int lSizeX = GetSizeX();
            int lSizeY = GetSizeY();
            auto* pBuffer = static_cast<uint32_t*>(ZUniMemory::Allocate(4 * lSizeX * lSizeY));
            Bitmap.GetRGBA(reinterpret_cast<uint8_t*>(pBuffer), 0, 0, lSizeX, lSizeY, 4 * lSizeX, 0);
            CreateMipLevel(0, lSizeX, lSizeY, pBuffer);
            ZUniMemory::Free(pBuffer);
        }
    }

    void ZBitmap::GetRGBA(uint8_t* pDst, int px, int py, int dx, int dy, int pitch, int dwLevel) const
    {
    }

    void ZBitmap::GetData(ZBitmap::TDataBlock* pData, int dwMipLevel)
    {
    }

    void ZBitmap::SetName(const char* pszName)
    {
        if (m_bNameAllocated)
        {
            ZUniMemory::Free(m_pszName);
            m_pszName = nullptr;
            m_bNameAllocated = false;
        }

        if (pszName)
        {
            m_pszName = static_cast<char*>(ZUniMemory::Allocate(static_cast<int>(strlen(pszName)) + 1));
            strcpy(m_pszName, pszName);
            m_bNameAllocated = true;
        }
        else
        {
            m_pszName = const_cast<char*>(s_EmptyString);
        }
    }

    int ZBitmap::GetSizeX() const
    {
        return m_dwSize[0];
    }

    int ZBitmap::GetSizeY() const
    {
        return m_dwSize[1];
    }

    void ZBitmap::SetChecksum(int lChecksum)
    {
        m_dwChecksum = lChecksum;
    }

    int ZBitmap::GetChecksum() const
    {
        return m_dwChecksum;
    }

    void ZBitmap::SetScaleFactor(float fScaleFactor)
    {
        m_fScaleFactor = fScaleFactor;
    }

    float ZBitmap::GetScaleFactor() const
    {
        return m_fScaleFactor;
    }

    int ZBitmap::GetMipLevelCount() const
    {
        return m_dwMipLevels;
    }

    ZBitmap::TMipLevel* ZBitmap::GetMIPLevel(int lMip)
    {
        return m_pMipLevels + lMip;
    }

    uint32_t ZBitmap::GetBinSize()
    {
        uint32_t dwSize = 0x1D;
        if (m_pszName)
        {
            dwSize = static_cast<uint32_t>(strlen(m_pszName)) + 0x1D;
        }

        for (int i = 0; i < GetMipLevelCount(); ++i)
        {
            dwSize += GetMIPLevel(i)->dwSize + 4;
        }

        return dwSize;
    }

    void ZBitmap::SaveBin(char* pBuffer)
    {
        *reinterpret_cast<uint32_t*>(pBuffer) = m_dwType;
        *reinterpret_cast<uint32_t*>(pBuffer + 4) = m_dwId;
        *reinterpret_cast<uint32_t*>(pBuffer + 8) = (static_cast<uint32_t>(GetSizeX()) << 16) | static_cast<uint32_t>(GetSizeY());
        *reinterpret_cast<uint32_t*>(pBuffer + 12) = static_cast<uint32_t>(GetMipLevelCount());
        *reinterpret_cast<uint32_t*>(pBuffer + 16) = static_cast<uint32_t>(GetParams());
        *reinterpret_cast<float*>(pBuffer + 20) = GetScaleFactor();
        *reinterpret_cast<uint32_t*>(pBuffer + 24) = static_cast<uint32_t>(GetChecksum());

        char* pDst = pBuffer + 28;
        if (m_pszName)
        {
            const size_t nameLen = strlen(m_pszName);
            memcpy(pDst, m_pszName, nameLen);
            pDst += nameLen;
        }
        *pDst = '\0';
        ++pDst;

        for (int i = 0; i < GetMipLevelCount(); ++i)
        {
            TMipLevel* pMipLevel = GetMIPLevel(i);
            *reinterpret_cast<uint32_t*>(pDst) = pMipLevel->dwSize;
            pDst += 4;
            memcpy(pDst, pMipLevel->pData, pMipLevel->dwSize);
            pDst += pMipLevel->dwSize;
        }
    }

    void ZBitmap::LoadBin(const char* pBuffer)
    {
        if (*reinterpret_cast<const uint32_t*>(pBuffer) != static_cast<uint32_t>(m_dwType))
        {
            ZASSERT(false);
        }

        m_dwId = *reinterpret_cast<const uint32_t*>(pBuffer + 4);

        const uint32_t dwPackedSize = *reinterpret_cast<const uint32_t*>(pBuffer + 8);
        SetSizeX(static_cast<int32_t>(dwPackedSize >> 16));
        SetSizeY(static_cast<int32_t>(dwPackedSize & 0xFFFF));
        SetMIPLevels(static_cast<int>(*reinterpret_cast<const uint32_t*>(pBuffer + 12)));
        SetParams(static_cast<int>(*reinterpret_cast<const uint32_t*>(pBuffer + 16)));
        SetScaleFactor(*reinterpret_cast<const float*>(pBuffer + 20));
        SetChecksum(static_cast<int>(*reinterpret_cast<const uint32_t*>(pBuffer + 24)));

        m_pszName = const_cast<char*>(pBuffer) + 28;

        const char* pSrc = pBuffer + 28 + strlen(pBuffer + 28) + 1;
        int lSizeX = GetSizeX();
        int lSizeY = GetSizeY();
        for (int i = 0; i < GetMipLevelCount(); ++i)
        {
            const uint32_t dwSize = *reinterpret_cast<const uint32_t*>(pSrc);
            pSrc += 4;
            SetData(const_cast<char*>(pSrc), static_cast<int>(dwSize), lSizeX, lSizeY, i, false);
            pSrc += dwSize;
            if (lSizeX > 1)
            {
                lSizeX >>= 1;
            }
            if (lSizeY > 1)
            {
                lSizeY >>= 1;
            }
        }
    }

    int ZBitmap::GetParams()
    {
        return m_dwParams;
    }

    void ZBitmap::SetParams(int lParams)
    {
        m_dwParams = lParams;
    }

    uint32_t ZBitmap::GetId() const
    {
        return m_dwId;
    }

    void ZBitmap::SetId(uint32_t lId)
    {
        m_dwId = lId;
    }

    void ZBitmap::SetData(void* pData, int dwSize, int dwSizeX, int dwSizeY, int dwLevel, bool bAllocate)
    {
        TMipLevel* pMipLevel = &m_pMipLevels[dwLevel];

        if (pMipLevel->bAllocated)
        {
            ZUniMemory::Free(pMipLevel->pData);
            pMipLevel->bAllocated = false;
        }

        if (bAllocate)
        {
            pMipLevel->bAllocated = true;
            pMipLevel->pData = ZUniMemory::Allocate(dwSize);
            memcpy(pMipLevel->pData, pData, dwSize);
        }
        else
        {
            pMipLevel->pData = pData;
        }

        pMipLevel->dwSize = dwSize;
        pMipLevel->dwSizeX = dwSizeX;
        pMipLevel->dwSizeY = dwSizeY;
    }

    void ZBitmap::SetMIPLevels(int lLevels)
    {
        for (int i = 0; i < GetMipLevelCount(); ++i)
        {
            if (m_pMipLevels[i].bAllocated)
            {
                ZUniMemory::Free(m_pMipLevels[i].pData);
            }
        }

        if (m_pMipLevels)
        {
            ZUniMemory::Free(m_pMipLevels);
        }

        m_dwMipLevels = lLevels;
        m_pMipLevels = static_cast<TMipLevel*>(ZUniMemory::Allocate(sizeof(TMipLevel) * lLevels));

        for (int i = 0; i < m_dwMipLevels; ++i)
        {
            m_pMipLevels[i].bAllocated = false;
            m_pMipLevels[i].pData = nullptr;
            m_pMipLevels[i].dwSize = 0;
        }
    }

    void ZBitmap::CreateMipLevels(uint32_t dwWidth, uint32_t dwHeight, bool bMipMap)
    {
        SetSizeX(static_cast<int32_t>(dwWidth));
        SetSizeY(static_cast<int32_t>(dwHeight));

        int lLevels = 1;
        if (bMipMap)
        {
            uint32_t lWidth = dwWidth >> 1;
            uint32_t lHeight = dwHeight >> 1;
            if (lWidth)
            {
                do
                {
                    if (!lHeight)
                    {
                        break;
                    }
                    lWidth >>= 1;
                    ++lLevels;
                    lHeight >>= 1;
                }
                while (lWidth);
            }
        }
        SetMIPLevels(lLevels);
    }

    void ZBitmap::Swizzle()
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

            auto* pSwizzled = static_cast<uint32_t*>(ZUniMemory::Allocate(4 * pMipLevel->dwSizeX * pMipLevel->dwSizeY));
            const auto* pSrc = static_cast<const uint32_t*>(pMipLevel->pData);

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

            memcpy(pMipLevel->pData, pSwizzled, 4 * pMipLevel->dwSizeX * pMipLevel->dwSizeY);
            ZUniMemory::Free(pSwizzled);
        }
    }

    void ZBitmap::SetSizeX(int32_t lX)
    {
        m_dwSize[0] = lX;
    }

    void ZBitmap::SetSizeY(int32_t lY)
    {
        m_dwSize[1] = lY;
    }

    void ZBitmap::SetMipLevelCount(int32_t lCount)
    {
        m_dwMipLevels = lCount;
    }

    void ZBitmap::CreateMipLevel(int level, int x1, int y1, const uint32_t* pRGBA)
    {
        PackMipLevel(level, x1, y1, pRGBA);

        const int lNextLevel = level + 1;
        if (lNextLevel == m_dwMipLevels)
        {
            return;
        }

        const int lSizeX = x1 > 1 ? x1 >> 1 : x1;
        const int lSizeY = y1 > 1 ? y1 >> 1 : y1;

        auto* pBuffer = static_cast<uint32_t*>(ZUniMemory::Allocate(4 * lSizeX * lSizeY));
        uint32_t* pDst = pBuffer;
        const uint8_t* pSrc = reinterpret_cast<const uint8_t*>(pRGBA);

        const int lHalfX = x1 >> 1;
        const int lHalfY = y1 >> 1;

        for (int y = 0; y < lSizeY; ++y)
        {
            for (int x = 0; x < lSizeX; ++x)
            {
                uint32_t dwPixel;
                if (lHalfX && lHalfY)
                {
                    const int lRowOffset = 8 * lHalfX;
                    const uint32_t dwR = (static_cast<uint32_t>(pSrc[0]) + pSrc[4] + pSrc[lRowOffset] + pSrc[lRowOffset + 4]) >> 2;
                    const uint32_t dwG = (static_cast<uint32_t>(pSrc[1]) + pSrc[5] + pSrc[lRowOffset + 1] + pSrc[lRowOffset + 5]) >> 2;
                    const uint32_t dwB = (static_cast<uint32_t>(pSrc[2]) + pSrc[6] + pSrc[lRowOffset + 2] + pSrc[lRowOffset + 6]) >> 2;
                    const uint32_t dwA = (static_cast<uint32_t>(pSrc[3]) + pSrc[7] + pSrc[lRowOffset + 3] + pSrc[lRowOffset + 7]) >> 2;
                    dwPixel = dwR | (dwG << 8) | (dwB << 16) | (dwA << 24);
                }
                else
                {
                    const uint32_t dwR = (static_cast<uint32_t>(pSrc[0]) + pSrc[4]) >> 1;
                    const uint32_t dwG = (static_cast<uint32_t>(pSrc[1]) + pSrc[5]) >> 1;
                    const uint32_t dwB = (static_cast<uint32_t>(pSrc[2]) + pSrc[6]) >> 1;
                    const uint32_t dwA = (static_cast<uint32_t>(pSrc[3]) + pSrc[7]) >> 1;
                    dwPixel = dwR | (dwG << 8) | (dwB << 16) | (dwA << 24);
                }
                *pDst++ = dwPixel;
                if (lHalfX)
                {
                    pSrc += 8;
                }
            }
            if (lHalfY)
            {
                pSrc += 8 * lHalfX;
            }
        }

        CreateMipLevel(lNextLevel, lSizeX, lSizeY, pBuffer);
        ZUniMemory::Free(pBuffer);
    }
}
